all:
	@(cd c; make)
	@(cd c++; make)
	@(cd c++1x; make)

clean cl:
	@(cd c; make clean)
	@(cd c++; make clean)
	@(cd c++1x; make clean)

rebuild rb: cl all

