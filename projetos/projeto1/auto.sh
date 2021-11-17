make
tmux send -t 0:2.1 "clear;./receiver /dev/ttyS1" C-m
tmux send -t 0:2.0 "clear;./emitter /dev/ttyS0" C-m
