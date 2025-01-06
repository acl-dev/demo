all: bin/pool
	@mkdir -p bin

bin/pool: pool/*.go
	@echo "build pool"
	@(go build -o bin/pool pool/*.go)

clean cl:
	@(rm -f bin/*)
