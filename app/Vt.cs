using System;
using System.Collections.Generic;
using System.Text;

namespace Luminal
{
    struct Cell
    {
        public int Ch;
        public uint Fg, Bg;
        public byte Attr;
        public string Marks;

        public const byte Bold = 1, Underline = 2, Inverse = 4, Dim = 8, Italic = 16, Wrapped = 32;
    }

    sealed class Vt
    {
        public readonly object Lock = new object();
        public int Rows { get; private set; }
        public int Cols { get; private set; }
        public int CursorRow, CursorCol;
        public bool CursorVisible = true;
        public bool AppCursorKeys, AppKeypad, BracketedPaste;
        public bool Dirty = true;
        public volatile string Title;
        readonly StringBuilder _osc = new StringBuilder();
        public Action<byte[]> Reply;

        const int MaxScrollback = 2000;
        readonly List<Cell[]> _scrollback = new List<Cell[]>();
        Cell[][] _main, _alt, _screen;
        bool _onAlt;
        int _top, _bottom;
        bool _wrapPending, _autoWrap = true, _insert, _originMode;
        bool[] _tabs;
        uint _fg, _bg;
        byte _attr;
        int _savedRow, _savedCol;
        uint _savedFg, _savedBg;
        byte _savedAttr;
        bool _lineDrawing, _g1LineDrawing, _useG1;
        int _lastCh = ' ';

        enum State { Ground, Esc, Csi, Osc, OscEsc, Charset, Skip }
        State _state;
        char _charsetSlot;
        readonly int[] _params = new int[32];
        int _paramCount;
        bool _hasParam;
        char _private, _intermediate;
        int _utfNeed, _utfCp;

        public Vt(int rows, int cols)
        {
            Rows = rows;
            Cols = cols;
            _main = NewGrid(rows, cols);
            _alt = NewGrid(rows, cols);
            _screen = _main;
            _bottom = rows - 1;
            ResetTabs();
        }

        static Cell[][] NewGrid(int rows, int cols)
        {
            var g = new Cell[rows][];
            for (int i = 0; i < rows; i++) g[i] = new Cell[cols];
            return g;
        }

        void ResetTabs()
        {
            _tabs = new bool[Cols];
            for (int i = 8; i < Cols; i += 8) _tabs[i] = true;
        }

        public int ScrollbackCount => _onAlt ? 0 : _scrollback.Count;
        public bool OnAltScreen => _onAlt;

        public Cell[] Line(int row)
        {
            if (row >= 0) return _screen[row];
            int i = _scrollback.Count + row;
            return i >= 0 ? _scrollback[i] : null;
        }

        void OscDone()
        {
            string s = _osc.ToString();
            _osc.Clear();
            if (s.StartsWith("0;") || s.StartsWith("2;")) Title = s.Substring(2);
        }

        public void Resize(int rows, int cols)
        {
            if (rows == Rows && cols == Cols) return;

            int drop = Math.Max(0, CursorRow - (rows - 1));
            for (int i = 0; i < drop; i++)
                if (!_onAlt) AddScrollback(_main[i]);
            _main = Reshape(_main, rows, cols, _onAlt ? 0 : drop);
            _alt = Reshape(_alt, rows, cols, _onAlt ? drop : 0);
            _screen = _onAlt ? _alt : _main;

            int restore = _onAlt || rows <= Rows ? 0 : Math.Min(rows - Rows, _scrollback.Count);
            if (restore > 0)
            {
                var keep = _main;
                _main = NewGrid(rows, cols);
                for (int r = 0; r < restore; r++)
                {
                    var old = _scrollback[_scrollback.Count - restore + r];
                    Array.Copy(old, _main[r], Math.Min(cols, old.Length));
                }
                for (int r = restore; r < rows; r++) Array.Copy(keep[r - restore], _main[r], cols);
                _scrollback.RemoveRange(_scrollback.Count - restore, restore);
                _screen = _main;
                CursorRow += restore;
            }
            Rows = rows;
            Cols = cols;
            CursorRow = Math.Min(CursorRow - drop, rows - 1);
            if (CursorRow < 0) CursorRow = 0;
            CursorCol = Math.Min(CursorCol, cols - 1);
            _top = 0;
            _bottom = rows - 1;
            _wrapPending = false;
            ResetTabs();
            Dirty = true;
        }

        static Cell[][] Reshape(Cell[][] old, int rows, int cols, int drop)
        {
            var g = NewGrid(rows, cols);
            for (int r = 0; r < rows && r + drop < old.Length; r++)
                Array.Copy(old[r + drop], g[r], Math.Min(cols, old[r + drop].Length));
            return g;
        }

        void AddScrollback(Cell[] line)
        {
            _scrollback.Add(line);
            if (_scrollback.Count > MaxScrollback) _scrollback.RemoveRange(0, 200);
        }

        public void Feed(byte[] buf, int n)
        {
            for (int i = 0; i < n; i++)
            {
                byte b = buf[i];
                if (_state == State.Ground && b >= 0x80)
                {
                    if (b >= 0xC0)
                    {
                        _utfNeed = b >= 0xF0 ? 3 : b >= 0xE0 ? 2 : 1;
                        _utfCp = b & (0x3F >> _utfNeed);
                    }
                    else if (_utfNeed > 0)
                    {
                        _utfCp = (_utfCp << 6) | (b & 0x3F);
                        if (--_utfNeed == 0) Print(_utfCp);
                    }
                    else Print(0xFFFD);
                    continue;
                }
                if (b < 0x80) _utfNeed = 0;
                Step(b);
            }
            Dirty = true;
        }

        void Step(byte b)
        {
            char c = (char)b;
            if (b < 0x20 && _state != State.Osc)
            {
                Control(b);
                return;
            }
            switch (_state)
            {
                case State.Ground:
                    Print(b);
                    break;
                case State.Esc:
                    EscDispatch(c);
                    break;
                case State.Charset:
                    if (_charsetSlot == '(') _lineDrawing = c == '0';
                    else _g1LineDrawing = c == '0';
                    _state = State.Ground;
                    break;
                case State.Skip:
                    _state = State.Ground;
                    break;
                case State.Csi:
                    if (c >= '0' && c <= '9')
                    {
                        if (_paramCount == 0) _paramCount = 1;
                        _params[_paramCount - 1] = Math.Min(_params[_paramCount - 1] * 10 + (c - '0'), 65535);
                        _hasParam = true;
                    }
                    else if (c == ';' || c == ':')
                    {
                        if (_paramCount == 0) _paramCount = 1;
                        if (_paramCount < _params.Length) _params[_paramCount++] = 0;
                    }
                    else if (c == '?' || c == '>' || c == '<' || c == '=') _private = c;
                    else if (c >= ' ' && c <= '/') _intermediate = c;
                    else if (c >= '@' && c <= '~')
                    {
                        _state = State.Ground;
                        CsiDispatch(c);
                    }
                    break;
                case State.Osc:
                    if (b == 7) { OscDone(); _state = State.Ground; }
                    else if (b == 0x1B) _state = State.OscEsc;
                    else if (_osc.Length < 256) _osc.Append((char)b);
                    break;
                case State.OscEsc:
                    OscDone();
                    _state = State.Ground;
                    break;
            }
        }

        void Control(byte b)
        {
            switch (b)
            {
                case 8:
                    _wrapPending = false;
                    if (CursorCol > 0) CursorCol--;
                    break;
                case 9:
                    _wrapPending = false;
                    do { CursorCol++; } while (CursorCol < Cols - 1 && !_tabs[CursorCol]);
                    if (CursorCol >= Cols) CursorCol = Cols - 1;
                    break;
                case 10: case 11: case 12:
                    Index();
                    break;
                case 13:
                    _wrapPending = false;
                    CursorCol = 0;
                    break;
                case 14: _useG1 = true; break;
                case 15: _useG1 = false; break;
                case 0x18: case 0x1A: _state = State.Ground; break;
                case 0x1B:
                    _state = State.Esc;
                    break;
            }
        }

        void EscDispatch(char c)
        {
            _state = State.Ground;
            switch (c)
            {
                case '[':
                    _state = State.Csi;
                    Array.Clear(_params, 0, _params.Length);
                    _paramCount = 0;
                    _hasParam = false;
                    _private = '\0';
                    _intermediate = '\0';
                    break;
                case ']': case 'P': case '_': case '^': case 'X':
                    _state = State.Osc;
                    break;
                case '(': case ')':
                    _charsetSlot = c;
                    _state = State.Charset;
                    break;
                case '#': case '%': case ' ': case '*': case '+':
                    _state = State.Skip;
                    break;
                case '7': SaveCursor(); break;
                case '8': RestoreCursor(); break;
                case 'D': Index(); break;
                case 'E': CursorCol = 0; Index(); break;
                case 'H': if (CursorCol < Cols) _tabs[CursorCol] = true; break;
                case 'M': ReverseIndex(); break;
                case 'c': FullReset(); break;
                case '=': AppKeypad = true; break;
                case '>': AppKeypad = false; break;
            }
        }

        void FullReset()
        {
            _fg = _bg = 0;
            _attr = 0;
            _top = 0;
            _bottom = Rows - 1;
            _autoWrap = true;
            _insert = _originMode = _wrapPending = false;
            AppCursorKeys = AppKeypad = BracketedPaste = false;
            CursorVisible = true;
            _lineDrawing = _g1LineDrawing = _useG1 = false;
            if (_onAlt) { _onAlt = false; _screen = _main; }
            _scrollback.Clear();
            for (int r = 0; r < Rows; r++) Array.Clear(_screen[r], 0, Cols);
            CursorRow = CursorCol = 0;
            ResetTabs();
        }

        void SaveCursor()
        {
            _savedRow = CursorRow;
            _savedCol = CursorCol;
            _savedFg = _fg;
            _savedBg = _bg;
            _savedAttr = _attr;
        }

        void RestoreCursor()
        {
            CursorRow = Math.Min(_savedRow, Rows - 1);
            CursorCol = Math.Min(_savedCol, Cols - 1);
            _fg = _savedFg;
            _bg = _savedBg;
            _attr = _savedAttr;
            _wrapPending = false;
        }

        int P(int i, int def) => i < _paramCount && _params[i] != 0 ? _params[i] : def;
        int Raw(int i) => i < _paramCount ? _params[i] : 0;

        void CsiDispatch(char c)
        {
            if (_private == '?')
            {
                if (c == 'h' || c == 'l')
                    for (int i = 0; i < Math.Max(_paramCount, 1); i++) PrivateMode(Raw(i), c == 'h');
                return;
            }
            if (_private != '\0' || _intermediate != '\0')
            {
                if (_private == '>' && c == 'c') Send("\x1b[>1;10;0c");
                return;
            }
            switch (c)
            {
                case 'A': MoveTo(Math.Max(CursorRow - P(0, 1), CursorRow >= _top ? _top : 0), CursorCol); break;
                case 'B': case 'e': MoveTo(Math.Min(CursorRow + P(0, 1), CursorRow <= _bottom ? _bottom : Rows - 1), CursorCol); break;
                case 'C': case 'a': MoveTo(CursorRow, Math.Min(CursorCol + P(0, 1), Cols - 1)); break;
                case 'D': MoveTo(CursorRow, Math.Max(CursorCol - P(0, 1), 0)); break;
                case 'E': MoveTo(Math.Min(CursorRow + P(0, 1), _bottom), 0); break;
                case 'F': MoveTo(Math.Max(CursorRow - P(0, 1), _top), 0); break;
                case 'G': case '`': MoveTo(CursorRow, Math.Min(P(0, 1) - 1, Cols - 1)); break;
                case 'd': MoveTo(Math.Min(P(0, 1) - 1 + (_originMode ? _top : 0), Rows - 1), CursorCol); break;
                case 'H': case 'f':
                    MoveTo(Math.Min(P(0, 1) - 1 + (_originMode ? _top : 0), Rows - 1), Math.Min(P(1, 1) - 1, Cols - 1));
                    break;
                case 'I': for (int i = P(0, 1); i > 0; i--) Control(9); break;
                case 'J': EraseDisplay(Raw(0)); break;
                case 'K': EraseLine(Raw(0)); break;
                case 'L': if (CursorRow >= _top && CursorRow <= _bottom) ScrollDown(CursorRow, _bottom, P(0, 1)); break;
                case 'M': if (CursorRow >= _top && CursorRow <= _bottom) ScrollUp(CursorRow, _bottom, P(0, 1), false); break;
                case 'P': DeleteChars(P(0, 1)); break;
                case '@': InsertChars(P(0, 1)); break;
                case 'X': ClearCells(CursorRow, CursorCol, Math.Min(CursorCol + P(0, 1), Cols)); break;
                case 'S': ScrollUp(_top, _bottom, P(0, 1), false); break;
                case 'T': ScrollDown(_top, _bottom, P(0, 1)); break;
                case 'b': for (int i = P(0, 1); i > 0; i--) Print(_lastCh); break;
                case 'c': Send("\x1b[?1;2c"); break;
                case 'g':
                    if (Raw(0) == 3) Array.Clear(_tabs, 0, _tabs.Length);
                    else if (CursorCol < Cols) _tabs[CursorCol] = false;
                    break;
                case 'h': if (Raw(0) == 4) _insert = true; break;
                case 'l': if (Raw(0) == 4) _insert = false; break;
                case 'm': Sgr(); break;
                case 'n':
                    if (Raw(0) == 6) Send("\x1b[" + (CursorRow + 1) + ";" + (CursorCol + 1) + "R");
                    else if (Raw(0) == 5) Send("\x1b[0n");
                    break;
                case 'r':
                    {
                        int t = P(0, 1) - 1, b = Math.Min(P(1, Rows), Rows) - 1;
                        if (t < b)
                        {
                            _top = t;
                            _bottom = b;
                            MoveTo(_originMode ? _top : 0, 0);
                        }
                        break;
                    }
                case 's': SaveCursor(); break;
                case 'u': RestoreCursor(); break;
                case 't':
                    if (Raw(0) == 18) Send("\x1b[8;" + Rows + ";" + Cols + "t");
                    break;
            }
        }

        void PrivateMode(int mode, bool on)
        {
            switch (mode)
            {
                case 1: AppCursorKeys = on; break;
                case 6: _originMode = on; MoveTo(on ? _top : 0, 0); break;
                case 7: _autoWrap = on; break;
                case 25: CursorVisible = on; break;
                case 2004: BracketedPaste = on; break;
                case 47: case 1047: SwitchScreen(on, false); break;
                case 1048: if (on) SaveCursor(); else RestoreCursor(); break;
                case 1049: SwitchScreen(on, true); break;
            }
        }

        void SwitchScreen(bool alt, bool saveCursor)
        {
            if (alt == _onAlt) return;
            if (alt && saveCursor) SaveCursor();
            _onAlt = alt;
            _screen = alt ? _alt : _main;
            if (alt)
                for (int r = 0; r < Rows; r++) Array.Clear(_alt[r], 0, Cols);
            if (!alt && saveCursor) RestoreCursor();
            _wrapPending = false;
        }

        void MoveTo(int row, int col)
        {
            CursorRow = Math.Max(0, row);
            CursorCol = Math.Max(0, col);
            _wrapPending = false;
        }

        void Send(string s) => Reply?.Invoke(Encoding.UTF8.GetBytes(s));

        static readonly string LineChars = "◆▒␉␌␍␊°±␤␋┘┐┌└┼⎺⎻─⎼⎽├┤┴┬│≤≥π≠£·";

        void Print(int cp)
        {
            if (cp >= 0x60 && cp < 0x7F && (_useG1 ? _g1LineDrawing : _lineDrawing)) cp = LineChars[cp - 0x60];
            int w = Width(cp);
            if (w == 0)
            {
                AddMark(cp);
                return;
            }
            _lastCh = cp;
            if (_wrapPending || CursorCol + w > Cols)
            {
                if (_autoWrap)
                {
                    _screen[CursorRow][Cols - 1].Attr |= Cell.Wrapped;
                    CursorCol = 0;
                    Index();
                }
                else CursorCol = Math.Max(0, Cols - w);
                _wrapPending = false;
            }
            if (_insert) InsertChars(w);
            var line = _screen[CursorRow];
            line[CursorCol].Ch = cp;
            line[CursorCol].Fg = _fg;
            line[CursorCol].Bg = _bg;
            line[CursorCol].Attr = _attr;
            line[CursorCol].Marks = null;
            if (w == 2)
            {
                line[CursorCol + 1].Ch = -1;
                line[CursorCol + 1].Fg = _fg;
                line[CursorCol + 1].Bg = _bg;
                line[CursorCol + 1].Attr = _attr;
            }
            CursorCol += w;
            if (CursorCol >= Cols)
            {
                CursorCol = Cols - 1;
                _wrapPending = true;
            }
        }

        void AddMark(int cp)
        {
            if (cp >= 0x200B && cp <= 0x200F) return;
            int c = _wrapPending ? CursorCol : CursorCol - 1;
            if (c < 0) return;
            var line = _screen[CursorRow];
            if (line[c].Ch == -1 && c > 0) c--;
            if (line[c].Ch <= 0 || (line[c].Marks != null && line[c].Marks.Length >= 8)) return;
            line[c].Marks += char.ConvertFromUtf32(cp);
        }

        static int Width(int cp)
        {
            if (cp < 0x300) return 1;
            if ((cp >= 0x300 && cp <= 0x36F) || (cp >= 0x200B && cp <= 0x200F) || (cp >= 0xFE00 && cp <= 0xFE0F) || (cp >= 0x20D0 && cp <= 0x20FF)) return 0;
            if ((cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2E80 && cp <= 0x303E) || (cp >= 0x3041 && cp <= 0xA4CF) || (cp >= 0xAC00 && cp <= 0xD7A3) ||
                (cp >= 0xF900 && cp <= 0xFAFF) || (cp >= 0xFE30 && cp <= 0xFE6F) || (cp >= 0xFF00 && cp <= 0xFF60) || (cp >= 0xFFE0 && cp <= 0xFFE6) ||
                (cp >= 0x1F300 && cp <= 0x1F64F) || (cp >= 0x1F900 && cp <= 0x1F9FF) || (cp >= 0x20000 && cp <= 0x3FFFD)) return 2;
            return 1;
        }

        void Index()
        {
            _wrapPending = false;
            if (CursorRow == _bottom) ScrollUp(_top, _bottom, 1, true);
            else if (CursorRow < Rows - 1) CursorRow++;
        }

        void ReverseIndex()
        {
            _wrapPending = false;
            if (CursorRow == _top) ScrollDown(_top, _bottom, 1);
            else if (CursorRow > 0) CursorRow--;
        }

        void ScrollUp(int top, int bottom, int n, bool keep)
        {
            n = Math.Min(n, bottom - top + 1);
            for (int i = 0; i < n; i++)
            {
                var old = _screen[top];
                if (keep && !_onAlt && top == 0)
                {
                    AddScrollback(old);
                    old = new Cell[Cols];
                }
                else Array.Clear(old, 0, old.Length);
                for (int r = top; r < bottom; r++) _screen[r] = _screen[r + 1];
                _screen[bottom] = old;
                FillBg(old);
            }
        }

        void ScrollDown(int top, int bottom, int n)
        {
            n = Math.Min(n, bottom - top + 1);
            for (int i = 0; i < n; i++)
            {
                var old = _screen[bottom];
                Array.Clear(old, 0, old.Length);
                for (int r = bottom; r > top; r--) _screen[r] = _screen[r - 1];
                _screen[top] = old;
                FillBg(old);
            }
        }

        void FillBg(Cell[] line)
        {
            if (_bg == 0) return;
            for (int i = 0; i < line.Length; i++) line[i].Bg = _bg;
        }

        void ClearCells(int row, int from, int to)
        {
            var line = _screen[row];
            for (int i = from; i < to; i++)
            {
                line[i].Ch = 0;
                line[i].Fg = 0;
                line[i].Bg = _bg;
                line[i].Attr = 0;
            }
        }

        void EraseDisplay(int mode)
        {
            _wrapPending = false;
            if (mode == 0)
            {
                ClearCells(CursorRow, CursorCol, Cols);
                for (int r = CursorRow + 1; r < Rows; r++) ClearCells(r, 0, Cols);
            }
            else if (mode == 1)
            {
                for (int r = 0; r < CursorRow; r++) ClearCells(r, 0, Cols);
                ClearCells(CursorRow, 0, Math.Min(CursorCol + 1, Cols));
            }
            else if (mode == 2)
            {
                for (int r = 0; r < Rows; r++) ClearCells(r, 0, Cols);
            }
            else if (mode == 3 && !_onAlt) _scrollback.Clear();
        }

        void EraseLine(int mode)
        {
            _wrapPending = false;
            if (mode == 0) ClearCells(CursorRow, CursorCol, Cols);
            else if (mode == 1) ClearCells(CursorRow, 0, Math.Min(CursorCol + 1, Cols));
            else ClearCells(CursorRow, 0, Cols);
        }

        void DeleteChars(int n)
        {
            n = Math.Min(n, Cols - CursorCol);
            var line = _screen[CursorRow];
            Array.Copy(line, CursorCol + n, line, CursorCol, Cols - CursorCol - n);
            ClearCells(CursorRow, Cols - n, Cols);
            _wrapPending = false;
        }

        void InsertChars(int n)
        {
            n = Math.Min(n, Cols - CursorCol);
            var line = _screen[CursorRow];
            Array.Copy(line, CursorCol, line, CursorCol + n, Cols - CursorCol - n);
            ClearCells(CursorRow, CursorCol, CursorCol + n);
            _wrapPending = false;
        }

        static readonly uint[] Base16 =
        {
            0xFF1C1C1C, 0xFFD75F5F, 0xFF5FAF5F, 0xFFD7AF5F, 0xFF5F87D7, 0xFFAF5FAF, 0xFF5FAFAF, 0xFFC6C6C6,
            0xFF6C6C6C, 0xFFFF8787, 0xFF87D787, 0xFFFFD787, 0xFF87AFFF, 0xFFD787D7, 0xFF87D7D7, 0xFFEEEEEE,
        };

        public static uint Palette(int i)
        {
            if (i < 16) return Base16[i];
            if (i < 232)
            {
                i -= 16;
                int r = i / 36, g = (i / 6) % 6, b = i % 6;
                uint R = (uint)(r == 0 ? 0 : 55 + r * 40), G = (uint)(g == 0 ? 0 : 55 + g * 40), B = (uint)(b == 0 ? 0 : 55 + b * 40);
                return 0xFF000000 | (R << 16) | (G << 8) | B;
            }
            uint v = (uint)(8 + (i - 232) * 10);
            return 0xFF000000 | (v << 16) | (v << 8) | v;
        }

        void Sgr()
        {
            if (_paramCount == 0)
            {
                _fg = _bg = 0;
                _attr = 0;
                return;
            }
            for (int i = 0; i < _paramCount; i++)
            {
                int p = _params[i];
                if (p == 0) { _fg = _bg = 0; _attr = 0; }
                else if (p == 1) _attr |= Cell.Bold;
                else if (p == 2) _attr |= Cell.Dim;
                else if (p == 3) _attr |= Cell.Italic;
                else if (p == 4) _attr |= Cell.Underline;
                else if (p == 7) _attr |= Cell.Inverse;
                else if (p == 22) _attr &= unchecked((byte)~(Cell.Bold | Cell.Dim));
                else if (p == 23) _attr &= unchecked((byte)~Cell.Italic);
                else if (p == 24) _attr &= unchecked((byte)~Cell.Underline);
                else if (p == 27) _attr &= unchecked((byte)~Cell.Inverse);
                else if (p >= 30 && p <= 37) _fg = Base16[p - 30];
                else if (p == 39) _fg = 0;
                else if (p >= 40 && p <= 47) _bg = Base16[p - 40];
                else if (p == 49) _bg = 0;
                else if (p >= 90 && p <= 97) _fg = Base16[p - 90 + 8];
                else if (p >= 100 && p <= 107) _bg = Base16[p - 100 + 8];
                else if (p == 38 || p == 48)
                {
                    uint col = 0;
                    if (i + 2 < _paramCount && _params[i + 1] == 5)
                    {
                        col = Palette(Math.Min(_params[i + 2], 255));
                        i += 2;
                    }
                    else if (i + 4 < _paramCount && _params[i + 1] == 2)
                    {
                        col = 0xFF000000 | ((uint)(_params[i + 2] & 255) << 16) | ((uint)(_params[i + 3] & 255) << 8) | (uint)(_params[i + 4] & 255);
                        i += 4;
                    }
                    else continue;
                    if (p == 38) _fg = col;
                    else _bg = col;
                }
            }
        }
    }
}
