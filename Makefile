

clean:
	find . -name '.*.swp' -o -name '*~' | xargs rm 
	cd src && make clean && cd ../;
