# RLE vs halcon



## INTRODUCTION

RLE（run-length encoding） vs halcon 



## PLATFORM & BENCHMARK

### windows10 /window7 

- dependency : vs2013/halcon10/opencv2.4.11 

- build:

  open blob.sln file 

<details><summary><b>BENCHMARK</b></summary>

CPU: i5

4096*3000/threshold=(130,255)/NumRuns=34176/Area=5172496

|           |        | Feature Size | RLE     | Halcon | OpenCV    |
| --------- | ------ | ------------ | ------- | ------ | --------- |
| rectangle | erode  | 10*10        | 2.21ms  | 1.09ms | 11.60ms   |
|           |        | 20*20        | 1.19ms  | 0.59ms | 20.01ms   |
|           |        | 30*30        | 1.09ms  | 0.56ms | 30.73ms   |
|           |        | 40*40        | 1.24ms  | 0.61ms | 39.68ms   |
|           |        | 50*50        | 0.99ms  | 0.45ms | 58.47ms   |
|           |        | 60*60        | 0.93ms  | 0.41ms | 68.51ms   |
|           |        | 70*70        | 1.02ms  | 0.43ms | 80.86ms   |
|           |        | 80*80        | 0.96ms  | 0.43ms | 92.55ms   |
| rectangle | dilate | 10*10        | 2.44ms  | 0.80ms | 11.37ms   |
|           |        | 20*20        | 1.89ms  | 0.63ms | 20.09ms   |
|           |        | 30*30        | 1.25ms  | 0.35ms | 29.90ms   |
|           |        | 40*40        | 1.40ms  | 0.40ms | 39.72ms   |
|           |        | 50*50        | 1.32ms  | 0.36ms | 57.60ms   |
|           |        | 60*60        | 1.27ms  | 0.34ms | 70.91ms   |
|           |        | 70*70        | 1.38ms  | 0.39ms | 79.12ms   |
|           |        | 80*80        | 1.34ms  | 0.33ms | 93.08ms   |
| circle    | erode  | 10*10        | 3.66ms  | 4.19ms | 35.89ms   |
|           |        | 20*20        | 4.44ms  | 3.60ms | 127.00ms  |
|           |        | 30*30        | 5.41ms  | 4.89ms | 277.72ms  |
|           |        | 40*40        | 6.61ms  | 6.03ms | 495.59ms  |
|           |        | 50*50        | 8.36ms  | 6.05ms | 774.77ms  |
|           |        | 60*60        | 9.68ms  | 5.93ms | 1228.18ms |
|           |        | 70*70        | 10.41ms | 6.54ms | 1707.16ms |
|           |        | 80*80        | 11.88ms | 6.85ms | 2225.07ms |
| circle    | dilate | 10*10        | 4.31ms  | 1.9ms  | 36.01ms   |
|           |        | 20*20        | 7.41ms  | 2.91ms | 127.41ms  |
|           |        | 30*30        | 8.04ms  | 3.06ms | 279.09ms  |
|           |        | 40*40        | 9.70ms  | 3.68ms | 493.52ms  |
|           |        | 50*50        | 12.62ms | 4.17ms | 778.23ms  |
|           |        | 60*60        | 16.31ms | 4.74ms | 1235.95ms |
|           |        | 70*70        | 19.91ms | 5.32ms | 1714.93ms |
|           |        | 80*80        | 21.33ms | 5.80ms | 2237.01ms |



4096*3000/Region1=(100,200)/NumRuns=109252/Area=504717

|              | Region2 | RLE    | Halcon | NumRuns | Area   |
| ------------ | ------- | ------ | ------ | ------- | ------ |
| union        | 150,250 | 0.73ms | 0.55ms | 78599   | 345726 |
|              | 160,250 | 0.74ms | 0.54ms | 78952   | 328137 |
|              | 170,250 | 0.83ms | 0.54ms | 79294   | 310537 |
|              | 180,250 | 0.74ms | 0.54ms | 79725   | 292952 |
|              | 190,250 | 0.82ms | 0.54ms | 80254   | 274332 |
|              | 200,250 | 0.72ms | 0.52ms | 80895   | 254410 |
|              | 210,250 | 0.86ms | 0.67ms | 81126   | 230854 |
|              | 220,250 | 0.90ms | 0.70ms | 78544   | 200577 |
| difference   | 150,250 | 0.75ms | 0.85ms | 78599   | 345726 |
|              | 160,250 | 0.74ms | 0.85ms | 78952   | 328137 |
|              | 170,250 | 0.74ms | 0.83ms | 79294   | 310537 |
|              | 180,250 | 0.77ms | 0.82ms | 79725   | 292952 |
|              | 190,250 | 0.7ms  | 0.72ms | 80254   | 274332 |
|              | 200,250 | 0.63ms | 0.66ms | 80895   | 254410 |
|              | 210,250 | 0.63ms | 0.67ms | 81126   | 230854 |
|              | 220,250 | 0.67ms | 0.67ms | 78544   | 200577 |
| intersection | 150,250 | 0.85ms | 0.59ms | 78599   | 345726 |
|              | 160,250 | 0.87ms | 0.63ms | 78952   | 328137 |
|              | 170,250 | 0.87ms | 0.57ms | 79294   | 310537 |
|              | 180,250 | 0.86ms | 0.63ms | 79725   | 292952 |
|              | 190,250 | 0.84ms | 0.54ms | 80254   | 274332 |
|              | 200,250 | 0.69ms | 0.38ms | 80895   | 254410 |
|              | 210,250 | 0.66ms | 0.36ms | 81126   | 230854 |
|              | 220,250 | 0.69ms | 0.42ms | 78544   | 200577 |

4096*3000

|           | Region2 | RLE    | Halcon | NumRuns | Area    |
| --------- | ------- | ------ | ------ | ------- | ------- |
| connetion | 10,150  | 2.42ms | 0.38ms | 32977   | 7157170 |
|           | 10,160  | 2.41ms | 0.37ms | 33315   | 7174755 |
|           | 10,170  | 2.58ms | 0.41ms | 33684   | 7192405 |
|           | 10,180  | 2.49ms | 0.47ms | 34112   | 7209988 |
|           | 10,190  | 2.51ms | 0.40ms | 34708   | 7228708 |
|           | 10,200  | 2.70ms | 0.45ms | 35456   | 7248905 |
|           | 10,210  | 2.59ms | 0.55ms | 36487   | 7272974 |
|           | 10,220  | 2.67ms | 0.57ms | 37421   | 7303874 |



4096*3000

|              | Region2 | RLE    | Halcon | NumRuns | Area    |
| ------------ | ------- | ------ | ------ | ------- | ------- |
| select_shape | 10,150  | 0.36ms | 0.05ms | 32977   | 7157170 |
|              | 10,160  | 0.37ms | 0.06ms | 33315   | 7174755 |
|              | 10,170  | 0.39ms | 0.07ms | 33684   | 7192405 |
|              | 10,180  | 0.40ms | 0.09ms | 34112   | 7209988 |
|              | 10,190  | 0.41ms | 0.10ms | 34708   | 7228708 |
|              | 10,200  | 0.44ms | 0.15ms | 35456   | 7248905 |
|              | 10,210  | 0.47ms | 0.14ms | 36487   | 7272974 |
|              | 10,220  | 0.49ms | 0.17ms | 37421   | 7303874 |



2050*2448

|           | Region2 | RLE    | Halcon | NumRuns | Area    |
| --------- | ------- | ------ | ------ | ------- | ------- |
| threshold | 10,150  | 1.7ms  | 0.73ms | 53766   | 3614681 |
|           | 10,160  | 1.81ms | 0.76ms | 54162   | 3847323 |
|           | 10,170  | 2.04ms | 0.99ms | 60970   | 4109554 |
|           | 10,180  | 2.14ms | 1.00ms | 61838   | 4409065 |
|           | 10,190  | 1.88ms | 1.02ms | 47003   | 4666964 |
|           | 10,200  | 1.57ms | 0.61ms | 25647   | 4812658 |
|           | 10,210  | 1.43ms | 0.43ms | 13706   | 4867448 |
|           | 10,220  | 1.31ms | 0.52ms | 5010    | 4890160 |

</details>



## Contact

<img src="./doc/weixin.jpg" style="zoom:50%;" />