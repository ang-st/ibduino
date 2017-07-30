### ibduino

``` gcc main.c -o ibduino -lm ```

### Test audio 

``` 
$ ./program | pacat --format u8 --rate 8000
$ ./program | aplay
```

### TODO
* [] parse expression
* [] chain function call following parsed ast
* [] ....



