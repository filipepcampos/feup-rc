make
tmux send -t 0:2.1 "clear;./application receiver 1" C-m
tmux send -t 0:2.0 "clear;./application emitter 0 trick_file" C-m
