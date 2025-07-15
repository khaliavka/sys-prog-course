TARGET = main

run: $(TARGET)
	LD_LIBRARY_PATH=$$PWD:$$LD_LIBRARY_PATH ./$(TARGET)

$(TARGET): main.o libcalc.so
	gcc $< -L. -lcalc -o $@

main.o: main.c
	gcc -c $< -o $@
     
libcalc.so: add.o sub.o mul.o div.o
	gcc -shared $^ -o $@ 

add.o: add.c
	gcc -c -o $@ $<

sub.o: sub.c
	gcc -c -o $@ $<

mul.o: mul.c
	gcc -c -o $@ $<

div.o: div.c
	gcc -c -o $@ $<

clean:
	rm -f *.o *.so $(TARGET)

clean2:
	rm -f *.o $(TARGET)

