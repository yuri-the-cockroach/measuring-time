##
# Measuring time
#
# @file
# @version 0.1

run:
	if [ ! -e time.o ]; then make build; fi
	./time.o

build: clean
	clang -O0 -lpthread -o time.o time.c

clean:
	@if [ -e time.o ]; then rm time.o; fi

# end
