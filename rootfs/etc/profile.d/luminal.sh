PS1='\[\033]0;\w\007\]\h:\w\$ '

if [ ! -e /var/lib/luminal-welcomed ]; then
    : > /var/lib/luminal-welcomed
    echo 'Welcome to Luminal. Your home folder is Documents\Luminal.'
    echo 'Install anything you like with apk.'
fi
