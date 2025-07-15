TARGET = main

$(TARGET): main.o libcalc.a
	gcc $^ -o $@

main.o: main.c
	gcc -c $< -o $@
     
libcalc.a: add.o sub.o mul.o div.o
	ar rcs $@ $^

add.o: add.c
	gcc -c -o $@ $<

sub.o: sub.c
	gcc -c -o $@ $<

mul.o: mul.c
	gcc -c -o $@ $<

div.o: div.c
	gcc -c -o $@ $<

clean:
	rm -f *.o *.a $(TARGET)

clean2:
	rm -f *.o $(TARGET)

