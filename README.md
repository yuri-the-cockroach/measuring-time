# THE SHITPOST

Значит решил я тут для своего пет проекта сделать функцию, чтобы замерять, сколько у меня времени уходит на одну операцию
Ну казалось бы задача тривиальная, просто замерить время до и после вызова, ну думаю я, проще не бывает, и функция рапер у меня уже есть
Как-раз на такой случай

```c
#include <sys/time.h>

long GetTimeMicS(void) {
    struct timeval tv = {0, 0};
    gettimeofday(&tv, 0);
    return tv.tv_sec * (long)1e6 + tv.tv_usec;
}
```

И после этого просто в самом коде сделать 

```c
long start = GetTimeMicS();
FunctionToMesure();
long end = GetTimeMicS();

long timeItTook = end - start;
```

Ну и казалось бы, на этом всё, вопрос решён! Ответ перед глазами!
Ну какая-то собака меня дёрнула и голос в голове спросил "А интересно, сколько времени уходит на вызов функции GetTimeMicS()"
...

И конечно же мне захотелось узнать ответ на этот вопрос... Ну не может же это быть настолько сложно, замерить сколько времени уходит на замер времени...

Всего-то нужно

```c
int main (){
    long start = GetTimeMicS();
    GetTimeMicS();
    long end = GetTimeMicS();

    long timeItTook = end - start;
    printf("Function took %ldus", timeItTook); 
}
```
Отлично! Это было просто. Теперь компилируем, вызываем и...

```
$ clang -o getTimeOfTheTime.o getTimeOfTheTime.c
...
> Function took 1us
```
...
Погоди, что-то тут не так... Почему так долго...

Может компилятор мешает?... добавлю ка я -O0, чтобы неповадно было...

```
$ clang -O0 -o getTimeOfTheTime.o getTimeOfTheTime.c
...
> Function took 1us
```

Не помогло...
Может точности не хватает?... Ну ладно, нужно всего-то тогда замерить его в нансекундах... подумаешь, расплюнуть 

Нука... Как там это сделать... Ага! В time.h уже есть нужная мне функция, всего-то нужно переписать мою под неё...
```c
#include <time.h>

long GetTimeNS(void) {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec * 1000000000 + time.tv_nsec;
}
```

```c
int main() {
    long start = GetTimeNS();
    GetTimeNS();
    long end = GetTimeNS();

    long timeItTook = end - start;
    printf("Function took %ldns", timeItTook); 
    return 0;
}
```

Окей, попробуем теперь...

```
$ clang -O0 -o getTimeOfTheTime.o getTimeOfTheTime.c
...
> Function took 40ns
```

Окей! Это было быстро... во всех смыслах...
Но как-то я не доверяю этому результату... В статистике, чем больше выборка, тем точнее результат, поэтому давай сделаем цикл и проверим среднее... Мы же умные!(нет)

Давай возьмём за размер выборки например 1000...
```c
int main() {
    for (int i = 0; i < 1000; i++) {
        long start = GetTimeNS();
        GetTimeNS();
        long end = GetTimeNS();

        long timeItTook = end - start;
        printf("Function took %ldns", timeItTook); 
    }
    return 0;
}
```

Ну давай посмотрим...

```
$ clang -O0 -o getTimeOfTheTime.o getTimeOfTheTime.c
...
> Function took 40ns
> Function took 39ns
> Function took 41ns
> Function took 38ns
> Function took 42ns
> Function took 39ns
> Function took 41ns
> Function took 40ns
> Function took 40ns
> Function took 41ns
...
```

...

Ну... это была ошибка...
Наверное нужно просто их сложить, потом разделить на размер выборки, и вывести уже это...

И заодно выкинуть размер выборки в отдельную переменную, а то чует жопа, повторять её часто придётся...

```c

const long SAMPLE_SIZE = 1000;

int main() {
    long sumOfTimes = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        long start = GetTimeNS();
        GetTimeNS();
        long end = GetTimeNS();
        sumOfTimes += end - start;
    }

    printf("Function took %ldns", sumOfTimes / SAMPLE_SIZE); 
    return 0;
}
```

Отлично, теперь посмотрим...

```
$ clang -O0 -o getTimeOfTheTime.o getTimeOfTheTime.c
...
> Function took 40ns
```

Ну наверное ожидаемо...

Но давай увеличим выборку, а то я чёт не доверяю этим цифрам...
Пускай будет 1,000,000,000 или если на человеческом, СТО!

```c
const long SAMPLE_SIZE = 1000000000;
...
```

```
$ clang -O0 -o getTimeOfTheTime.o getTimeOfTheTime.c
...
```

...

```
...
```

...
...
...

Более минуты спустя оно наконец-то вернуло ответ...

``` 
> Function took 40ns
```
...Наверное ожидаемо...

Но почему она так долго тянет... Может это можно как-то ускорить?
40 * СТО(миллиард) это же 40 секунд, а не 60+...

(Вы уже наверное заметили прикол 🤡)

а...

```c
long start = GetTimeNS();
GetTimeNS();
long end = GetTimeNS();

```
Зачем я функцию вызываю 3 раза...
Я же могу её дважды вызвать и получить нормальный результат... А так я получаю время двух вызовов этой же функции...

```c
...
long start = GetTimeNS();
long end = GetTimeNS();
...
```

А ещё наверное неплохо было бы замерить, сколько этот цикл работал, чтобы хоть как-то понять, что просиходит...

```c
...
long workTimeStart = GetTimeNS();
for (int i = 0; i < SAMPLE_SIZE; i++) {
    long start = GetTimeNS();
    GetTimeNS();
    long end = GetTimeNS();
    sumOfTimes += end - start;
}
long workTime = GetTimeNS() - workTimeStart;
...
```

А ещё как-то глаза болят от такого количества нуленей... Но в си нет более удобного способа репрезентации больших чисел...

В питоне например можно их разбить так:
```python
something: int = 1_000_000_000
```
И это валидный синтаксис полностью прозрачный для питона. Он только для людей...

Хотя в си можно писать числа с плавающей запятой через экспаненту...
```c
float something = 1e9;
```

Но почему-то с интами так нельзя... почему...
А что если...

```c
long something = (long)1e9;
```

...

Какая-же хуета...
Но оно работает...

Ну ок...

В итоге у нас получается что-то вроде:

```c

const long SAMPLE_SIZE = (long)1e9;

int main() {
    long sumOfTimes = 0;
    long workTimeStart = GetTimeNS();
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        long start = GetTimeNS();
        long end = GetTimeNS();
        sumOfTimes += end - start;
    }
    long workTime = GetTimeNS() - workTimeStart;
    printf("Average mesurment %ldns\n", sumOfTimes / SAMPLE_SIZE); 
    printf("The whole thing %ldns\n", workTime);
    return 0;
}
```

И получается у нас что-то вроде этого...

```
$ clang -O0 -o nottime.o nottime.c && ./nottime.o

> Average mesurment 18ns
The whole thing 36502397382ns
```

Ага! в 2 раза быстрее! Собственно чего и следовало ожидать.
Но вот только общее время вообще не читабельное...

Ну давай набросаю функцию, которая будет красиво выводить...

```c
void DisplayTime(const char *restrict text, long time) {
    printf("%s %lds:%.3ldms:%.3ldus:%.3ldns\n", text, time / (long)1e9,
           time / (long)1e6 % (long)1e3, time / (long)1e3 % (long)1e3,
           time % (long)1e3);
}
```

...Ну как-то так... Выглядит вообще шикарно конечно... Ну давай попробуем...

```
$ clang -O0 -o nottime.o nottime.c && ./nottime.o

> Average mesurment 18ns
The whole thing took: 36s:315ms:842us:687ns
```

Замечательно! Теперь даже почти понятно... Но как-то долго... Почему она работает в два раза дольше, чем одна операция * количество семплов?...

```c
...
for (int i = 0; i < SAMPLE_SIZE; i++) {
    long start = GetTimeNS();
    long end = GetTimeNS();
    sumOfTimes += end - start;
}
...
```

...а, ну да, логично, потому-что я её дважды вызываю
Не по царски это, нужно исправить... Если я получу время один раз, а потом запомню его на следующий цикл, то тогда мне нужно будет вызывать функцию только один раз за цикл...
а нук...
```c
...
long lastTime = 0;
long thisTime = GetTimeNS();
for (long i = 0; i < SAMPLE_SIZE; i++) {
    lastTime = thisTime;
    thisTime = GetTimeNS();
    sumOfTimes += (thisTime - lastTime);
}
...
```

```
$ clang -O0 -o nottime.o nottime.c && ./nottime.o

> Average mesurment 18ns
The whole thing took: 18s:241ms:957us:721ns
```

Ти подивися яка краса!

Практически идеальный скейлинг, лучше и не бывает... 🥲 ... или бывает?

Hear me out...

Выборка в СОТ(миллиард), это конечно хорошо, но может и больше попробуем? Кто его знает, может что-то и изменится...

План простой:

1. Мой процессор, как и любой современный процессор, имеет больше одного ядра и два потока на ядро. Если быть точнее, то у меня `AMD Ryzen 9 5900X (24) @ 5.16 GHz` 12 ядер 24 потока...
Да я хвастаюсь... что мне ещё с ним делать, время мерить? кстати идея...
2. Мы воспользуемся всеми потоками процессора
3. <redacted>
4. Profit!

Ну должно же быть не сложно, правда? Всего-то нужно паралельно измерить время в 24х процессах СТО(миллиард) раз, а потом сложить это всё...

Для этого мне пригодится библиотека `pthread`, она идеально подходит для моих целей, как будто была сделал для этого 🤔...

Сначала нужно получить количество потоков проца, и это на самом деле проще сказать, чем сделать, но в итоге я обнаружил, что это число можно получить из unistd.h с помощью одной строчки кода:

```c
...
int THREADS = sysconf(_SC_NPROCESSORS_ONLN);
...
```

Вы можете спросить: Нафига так париться? Почему-бы просто не захардкодить число потоков в программу? 
На что я отвечу: ...no... Это скучно...

Дальше нужно сделать несколько вещей:

1. Создать список, в котором будут храниться адреса каждого под-процесса
2. Создать список, куда будут записаны ответы
3. Вынести вычисления времени в отдельную функцию
4. Запустить всё это безобразие...

С первым пунктом всё просто:
```c
...
pthread_t threadsPtr[THREADS];
...
```
И дело с концом. Да серьёзно.

Со вторым по сложнее.
Я пытался сделать это так-же просто, но ничего не вышло, постоянно упирался в проблемы с сегфолтами и раздные приколы, поэтому в итоге я выродил это:

```c
...
long **data = calloc(THREADS, sizeof(long*));

for ( int i = 0; i < THREADS; i++ ) {
    data[i] = calloc(1, sizeof(long));
}
...
```

Это список с указателями на числа, потому-что числа придётся передавать по ссылке. Поток не может просто так вернуть его, потому-что он не связан с основным процессом, поэтому ему придётся писать напрямую в память.

С третьим всё не так уж и сложно. Нужно просто взять имеющийся код, перенести его в отдельную функцию как это было бы обычно, но нужно очесть, что функции потока все одлжны выглядить так: `void *func(void *arg)` поэтому придётся кастовать указатель на число в указатель без типа, а потом обратно, но в итоге получается что-то вроде этого:

```c
void* TimeThread(void *thread) {
    long timesSum = 0;
    long thisTime = GetTimeNS();
    long lastTime = 0;
    long *toReturn = (long *)thread;

    for (long i = 0; i < SAMPLE_SIZE; i++) {
        lastTime = thisTime;
        thisTime = GetTimeNS();
        timesSum += (thisTime - lastTime);
    }
    toReturn[0] = timesSum;
    return NULL;
}
```

Теперь остальсь запустить эти потоки:

```c
...
for ( long i = 0; i < THREADS; i++ ) {
    long result = pthread_create(&threadsPtr[i], NULL, TimeThread, (void *)data[i]);
}
...
```

Дальше встаёт вопрос: А как данные то получить...
Дело в том, что чтобы получить данные, нужно убедиться, что все потоки закончили свою работу, и только после этого можно их суммировать.

Для этого есть удобная функция: `pthread_join` она позволит просто подожать пока поток вернёт значение. Она блокирующая, но нас это устраивает.

```c
...
for (int i = 0; i < THREADS; i++) {
    long result_code = pthread_join(threadsPtr[i], NULL);
    assert(!result_code);
    printf("In main: Thread %d has ended. Result: ", i);
    DisplayTime("", data[i][0]);
    timesSum += data[i][0];
    printf("average time = %ldns\n", data[i][0] / SAMPLE_SIZE);
}
...
```

Добавить принтов в конец...

```c
...
printf("<-------------- All threads have finished ------------->\n");

printf("Mesured time %ld times. It took", SAMPLE_SIZE * THREADS);
DisplayTime("", timeEnd - timeStart);

DisplayTime("Time sum:", timesSum);
printf("average time = %ldns\n", average);
...
```

И в конце получается вот это недоразумение:

```c
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const long SAMPLE_SIZE = (long)1e9;

void DisplayTime(const char *restrict text, long time) {
    printf("%s %lds:%.3ldms:%.3ldus:%.3ldns\n", text, time / (long)1e9,
           time / (long)1e6 % (long)1e3, time / (long)1e3 % (long)1e3,
           time % (long)1e3);
}

long GetTimeNS(void) {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec * (long)1e9 + time.tv_nsec;
}

void* TimeThread(void *thread) {
    long sumOfTimes = 0;
    long thisTime = GetTimeNS();
    long lastTime = 0;
    long *toReturn = (long *)thread;

    for (long i = 0; i < SAMPLE_SIZE; i++) {
        lastTime = thisTime;
        thisTime = GetTimeNS();
        sumOfTimes += (thisTime - lastTime);
    }
    toReturn[0] = sumOfTimes;
    return NULL;
}

int main() {
    int THREADS = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t threadsPtr[THREADS];
    long sumOfTimes = 0;
    long workTimeStart = GetTimeNS();

    long **data = calloc(THREADS, sizeof(long*));

    for ( int i = 0; i < THREADS; i++ ) {
        data[i] = calloc(1, sizeof(long));
    }

    long timeStart = GetTimeNS();
    for ( long i = 0; i < THREADS; i++ ) {
        long result = pthread_create(&threadsPtr[i], NULL, TimeThread, (void *)data[i]);
    }

    for (int i = 0; i < THREADS; i++) {
        long result_code = pthread_join(threadsPtr[i], NULL);
        assert(!result_code);
        printf("In main: Thread %d has ended. Result: ", i);
        DisplayTime("", data[i][0]);
        sumOfTimes += data[i][0];
        printf("average time = %ldns\n", data[i][0] / SAMPLE_SIZE);
    }
    long timeEnd = GetTimeNS();
    long average = sumOfTimes / (SAMPLE_SIZE * THREADS);

    printf("<-------------- All threads have finished ------------->\n");

    printf("Mesured time %ld times. It took", SAMPLE_SIZE * THREADS);
    DisplayTime("", timeEnd - timeStart);

    DisplayTime("Time sum:", sumOfTimes);
    printf("average time = %ldns\n", average);
    return 0;
}
```

Я знаю... Оно прекрасно...

А теперь момент истины...

```
$ clang -O0 -lpthread -o nottime.o nottime.c && ./nottime.o

> Segmentation fault (core dumped) 
```

...блять... минутку... *Достаёт бубен*

```
$ clang -O0 -lpthread -o nottime.o nottime.c && ./nottime.o

>
In main: Thread 0 has ended. Result:  22s:377ms:328us:910ns
average time = 22ns
In main: Thread 1 has ended. Result:  22s:442ms:559us:129ns
average time = 22ns
In main: Thread 2 has ended. Result:  22s:401ms:419us:190ns
average time = 22ns
In main: Thread 3 has ended. Result:  22s:208ms:276us:483ns
average time = 22ns
In main: Thread 4 has ended. Result:  22s:331ms:580us:301ns
average time = 22ns
In main: Thread 5 has ended. Result:  22s:435ms:343us:116ns
average time = 22ns
In main: Thread 6 has ended. Result:  22s:533ms:064us:566ns
average time = 22ns
In main: Thread 7 has ended. Result:  22s:347ms:308us:395ns
average time = 22ns
In main: Thread 8 has ended. Result:  22s:483ms:018us:749ns
average time = 22ns
In main: Thread 9 has ended. Result:  22s:561ms:028us:922ns
average time = 22ns
In main: Thread 10 has ended. Result:  22s:342ms:851us:844ns
average time = 22ns
In main: Thread 11 has ended. Result:  22s:532ms:810us:124ns
average time = 22ns
In main: Thread 12 has ended. Result:  22s:740ms:421us:266ns
average time = 22ns
In main: Thread 13 has ended. Result:  22s:543ms:343us:628ns
average time = 22ns
In main: Thread 14 has ended. Result:  22s:413ms:854us:991ns
average time = 22ns
In main: Thread 15 has ended. Result:  22s:245ms:618us:912ns
average time = 22ns
In main: Thread 16 has ended. Result:  22s:535ms:424us:841ns
average time = 22ns
In main: Thread 17 has ended. Result:  22s:530ms:087us:812ns
average time = 22ns
In main: Thread 18 has ended. Result:  22s:657ms:569us:312ns
average time = 22ns
In main: Thread 19 has ended. Result:  22s:495ms:145us:765ns
average time = 22ns
In main: Thread 20 has ended. Result:  22s:441ms:197us:588ns
average time = 22ns
In main: Thread 21 has ended. Result:  22s:539ms:043us:844ns
average time = 22ns
In main: Thread 22 has ended. Result:  22s:379ms:321us:919ns
average time = 22ns
In main: Thread 23 has ended. Result:  22s:635ms:620us:793ns
average time = 22ns
<-------------- All threads have finished ------------->
Mesured time 24000000000 times. It took 22s:740ms:826us:945ns
Time sum: 539s:153ms:240us:400ns
average time = 22ns
```

Вот так вот... Оно работает! Оказалось чутка медленнее, чем на одном потоке, видимо из-за какой-то магии шаманской...

Зато теперь мы знаем наверняка, сколько времени занимает измерение времени:
18 наносекунд, если на одном потоке
22 наносекунды, если на нескольких
