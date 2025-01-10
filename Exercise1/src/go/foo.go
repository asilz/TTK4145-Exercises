package main

import (
    . "fmt"
    "runtime"
)

var i = 0

func incrementing(increment, read chan int) {
    for j := 0; j < 1000000; j++ {
        increment <- 1;
	}
    read <- 1;
}

func decrementing(decrement, read chan int) {
    for j := 0; j < 1000000; j++ {
        decrement <- 1;
	}
    read <- 1;
}

func main() {
    // What does GOMAXPROCS do? What happens if you set it to 1?
    runtime.GOMAXPROCS(2);    
	
    // TODO: Spawn both functions as goroutines
    increment := make(chan int);
    decrement := make(chan int);
    read := make(chan int);
	go incrementing(increment, read);
	go decrementing(decrement, read);
	
    // We have no direct way to wait for the completion of a goroutine (without additional synchronization of some sort)
    // We will do it properly with channels soon. For now: Sleep.
    finished := 0;
    for true {
    select{
    case <- increment:
        i++
    case <- decrement:
        i--
    case <- read:
        finished++;
        if(finished == 2){
            Println("The magic number is:", i);
            return;
        }
        
    }
    }
    return;
}