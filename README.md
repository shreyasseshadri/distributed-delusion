# DHT

## Steps to run

```
make
mpirun -n 4 ./dht <total_no_of_keys>
```
## Test Case
Inserting: 2, 44, 7, 65, 1, 43, 28, 49, 34, 55, 18, 20, 39, 14

Expected Output
```
💻  →  mpirun -n 4 ./dht 10
Inserting 2
Inserted 2 process: 0 position: 2
-----------------------------------------------------------------------------
Inserting 44
collission with value 2 at process: 0  position: 2
Inserted 44 process: 0 position: 3
-----------------------------------------------------------------------------
Inserting 7
Inserted 7 process: 1 position: 1
-----------------------------------------------------------------------------
Inserting 65
collission with value 2 at process: 0  position: 2
collission with value 44 at process: 0  position: 3
Inserted 65 process: 0 position: 4
-----------------------------------------------------------------------------
Inserting 1
Inserted 1 process: 0 position: 1
-----------------------------------------------------------------------------
Inserting 43
collission with value 1 at process: 0  position: 1
collission with value 2 at process: 0  position: 2
collission with value 44 at process: 0  position: 3
collission with value 65 at process: 0  position: 4
Inserted 43 process: 0 position: 5
-----------------------------------------------------------------------------
Inserting 28
collission with value 7 at process: 1  position: 1
Inserted 28 process: 1 position: 2
-----------------------------------------------------------------------------
Inserting 49
collission with value 7 at process: 1  position: 1
collission with value 28 at process: 1  position: 2
Inserted 49 process: 1 position: 3
-----------------------------------------------------------------------------
Inserting 34
Inserted 34 process: 2 position: 1
-----------------------------------------------------------------------------
Inserting 55
collission with value 34 at process: 2  position: 1
Inserted 55 process: 2 position: 2
-----------------------------------------------------------------------------
Inserting 18
Inserted 18 process: 3 position: 0
-----------------------------------------------------------------------------
Inserting 20
Inserted 20 process: 3 position: 2
-----------------------------------------------------------------------------
Inserting 39
collission with value 18 at process: 3  position: 0
Inserted 39 process: 3 position: 1
-----------------------------------------------------------------------------
Inserting 14
collission with value 55 at process: 2  position: 2
Inserted 14 process: 2 position: 3
-----------------------------------------------------------------------------

Retrieved 2 from process: 0 position: 2 
Retrieved 44 from process: 0 position: 3 
Retrieved 7 from process: 1 position: 1 
Retrieved 65 from process: 0 position: 4 
Retrieved 1 from process: 0 position: 1 
Retrieved 43 from process: 0 position: 5 
Retrieved 28 from process: 1 position: 2 
Retrieved 49 from process: 1 position: 3 
Retrieved 34 from process: 2 position: 1 
Retrieved 55 from process: 2 position: 2 
Retrieved 18 from process: 3 position: 0 
Retrieved 20 from process: 3 position: 2 
Retrieved 39 from process: 3 position: 1 
Retrieved 14 from process: 2 position: 3 

Number of collisions: 13
```