#!/bin/bash
make re
rm -rf ./.test
mkdir -p ./.test

calc_burnout(){
time=$(($compile + $cooldown))
if [ $(($coder%2)) -eq 0 ]
then
burnout=$(($time*2 + 10))
else
burnout=$(($time*3 + 10))
fi
}

testing(){

n=$1
coder=$2
compile=$4
refactor=$5
debug=$6
compilnb=$7
cooldown=$8
scheduler=$9
if [ $3 -eq -1 ]
then
calc_burnout
else
burnout=$3
fi


i=0
echo "test no $n: ./codexion $coder $burnout $compile $debug $refactor $compilnb $cooldown $scheduler"
while ((i <= 9)); do
    touch ./.test/test$n$i.test
    ./codexion $coder $burnout $compile $debug $refactor $compilnb $cooldown $scheduler > "./.test/test$n$i.test"
	if [ $(cat ./.test/test$n$i.test | grep compiling | wc -l) -ge $(($coder*$compilnb)) ]
	then
	echo -e "$i | $(cat ./.test/test$n$i.test | grep compiling | wc -l) | \e[32mok\e[0m"
	else
	echo -e "$i | $(cat ./.test/test$n$i.test | grep compiling | wc -l) | \e[31mko\e[0m .test/test$n$i.test"
	fi
    i=$((i+1))
	if [ $coder -ge 100 ]
	then
	sleep 2
	fi
done
burnout=-1
}
n=0

#standard values, edf
compile=60
refactor=60
debug=60
compilnb=7
cooldown=60
scheduler=edf
burnout=-1

n=$((n+1))
coder=10
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=11
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=100
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=101
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=200
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=201
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

# standard values, fifo
scheduler=fifo

n=$((n+1))
coder=10
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=11
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=100
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=101
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=200
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=201
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

# testing with 0, edf
compile=0
refactor=0
debug=0
compilnb=7
cooldown=0
scheduler=edf

n=$((n+1))
coder=10
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=101
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

#testing with 0, fifo

scheduler=fifo
n=$((n+1))
coder=11
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=100
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

#testing random values,
compile=10
refactor=10
debug=10
compilnb=100
cooldown=10
scheduler=edf

n=$((n+1))
coder=10
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

compile=100
refactor=0
debug=0
compilnb=2
cooldown=1
scheduler=fifo

n=$((n+1))
coder=20
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

compile=0
refactor=10
debug=10
compilnb=2
cooldown=10
scheduler=fifo


#incorrect inputs
compile=60
refactor=60
debug=60
compilnb=7
cooldown=60
scheduler=edf

n=$((n+1))
coder=0
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

compile=60
refactor=60
debug=60
compilnb=0
cooldown=60
scheduler=edf

n=$((n+1))
coder=10
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=-1
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=10
debug=-1
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler

n=$((n+1))
coder=10
debug=60
refactor=2147483648
testing $n $coder $burnout $compile $refactor $debug $compilnb $cooldown $scheduler