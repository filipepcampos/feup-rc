make
tmux send -t 0:2.0 "./emitter /dev/ttyS0" C-m
tmux send -t 0:2.1 "./receiver /dev/ttyS1" C-m
