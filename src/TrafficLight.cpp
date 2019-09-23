#include <iostream>
#include <random>
#include <queue>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
  	std::unique_lock<std::mutex> uLock(_mutex);
  	_cond.wait(uLock, [this] { return !_queue.empty(); });
  	T msg = std::move(_queue.back());
  	_queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
	std::lock_guard<std::mutex> uLock(_mutex);
  _queue.push_back(std::move(msg));
	_cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
  	_message = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
	while(true){
      //reduce the CPU usage
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      TrafficLightPhase curr = _message->receive();
      if(curr==green) return ;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  
    std::random_device rd;
  	std::mt19937 eng(rd());
  	std::uniform_int_distribution<> distr(4000, 6000);
  	int cycleDuration = distr(eng);
  
  	auto lastUpdate = std::chrono::system_clock::now();
  	while(true){
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
      
	    if (timeSinceLastUpdate >= cycleDuration){
        
        if(_currentPhase==red){
         	_currentPhase = green; 
        }
        else{
          	_currentPhase = red; 
        }
        
        auto message = _currentPhase;
        auto sent = std::async(std::launch::async,&MessageQueue<TrafficLightPhase>::send,_message,std::move(message));
        sent.wait();
        cycleDuration = distr(eng);
        lastUpdate = std::chrono::system_clock::now();
      }
      
    }
}

