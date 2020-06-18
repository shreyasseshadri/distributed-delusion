# DHT

## Steps to run test

```
make
mpirun -n 4 ./dht <hash_table_size> <no_of_unique_values> <no_of_same_hashes_for_each_unique_value> 
```

## Sample test cases
```
💻  →  mpirun -n 4 ./test 16 5 3   
LP get time: 4.267000 ms
CLP get time: 2.031000 ms
CLP Collisions: 75
LP Collisions: 75

💻  →  mpirun -n 4 ./test 400 90 4
LP get time: 734.806000 ms
CLP get time: 228.511000 ms
CLP Collisions: 48600
LP Collisions: 48600

💻  →  mpirun -n 4 ./test 440 100 4
LP get time: 504.936000 ms
CLP get time: 407.870000 ms
CLP Collisions: 60000
LP Collisions: 60000
```
