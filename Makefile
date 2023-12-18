main:
	gcc days.c -o days -O3
	gcc solver_solutions.c -o solver -O3
	gcc no_solutions.c -o no_solutions -O3

old:
	gcc old_solver.c -o old_solver -O3

clean:
	rm -f days solver no_solutions