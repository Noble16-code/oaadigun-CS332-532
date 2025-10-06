FILE = oaadigun_HW02

build: $(FILE).c
	# compile with warnings, debug info, and math library
	gcc -Wall -g $(FILE).c -o $(FILE) -lm -fno-pie -no-pie

.PHONY: db

db:
	gdb -tui $(FILE)

run:
	./$(FILE)
