#ifndef THREAD_TIMMER_HPP
#define THREAD_TIMMER_HPP

#include <thread>
#include <chrono>
#include <functional>
#include <iostream>

class threadTimer {
	// If we should stop the current timer
	bool clear = false;
	int currentIndex = 0;
	// The current timer
	std::thread currentTimer;

public:
	~threadTimer(){
		// If the thread is joinable... detach it
		if(currentTimer.joinable())
			currentTimer.detach();

		clear = false;
	}

	template<typename Function, typename DurationRep, typename DurationPeriod>
    void setTimeout(Function function, std::chrono::duration<DurationRep, DurationPeriod> delay){
		// Stop the old timer
		stopAndWait();

		clear = false;
		currentTimer = std::thread([=, myIndex = ++this->currentIndex]() {
	        if(myIndex == currentIndex && clear) return;
	        std::this_thread::sleep_for(delay);
	        if(myIndex == currentIndex && clear) return;
	        function();
	    });
	}

    template<typename Function, typename DurationRep, typename DurationPeriod>
    void setInterval(Function function, std::chrono::duration<DurationRep, DurationPeriod> interval){
		// Stop the old timer
		stopAndWait();

		clear = false;
		currentIndex++;
	   	currentTimer = std::thread([=, myIndex = ++this->currentIndex]() {
	        while(true) {
	            if(myIndex == currentIndex && clear) return;
	            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	            if(myIndex == currentIndex && clear) return;
	            function();
	        }
	    });
	}

	// Stop the timer
    void stop(){
		clear = true;
	}

	// Wait for the timer to finish
	void wait(){
		if(currentTimer.joinable())
			currentTimer.join();
	}

	// Stop the timer and wait for it to finish
	inline void stopAndWait(){
		stop();
		wait();
	}
};

#endif /* end of include guard: THREAD_TIMMER_HPP */
