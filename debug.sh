gcc src/trail.c -o vidas_secas -g -I./include -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
gdb ./vidas_secas