all: parent child

parent: parent.c
	gcc parent.c -o parent -pthread

child: child.c
	gcc child.c -o child -pthread

clean:
	rm -f parent child output.txt
