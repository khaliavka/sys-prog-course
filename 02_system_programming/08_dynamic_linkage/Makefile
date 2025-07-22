TARGET = main

.PHONY: run clean clean2

run: $(TARGET)
	LD_LIBRARY_PATH=$$PWD:$$LD_LIBRARY_PATH ./$(TARGET)

$(TARGET): main.o libcalc.so
	gcc $< -L. -lcalc -o $@

main.o: main.c
	gcc -c $< -o $@
     
libcalc.so: add.o sub.o mul.o div.o
	gcc -shared $^ -o $@ 

add.o: add.c
	gcc -c -fPIC -o $@ $<

sub.o: sub.c
	gcc -c -fPIC -o $@ $<

mul.o: mul.c
	gcc -c -fPIC -o $@ $<

div.o: div.c
	gcc -c -fPIC -o $@ $<

clean:
	rm -f *.o *.so $(TARGET)

clean2:
	rm -f *.o $(TARGET)

