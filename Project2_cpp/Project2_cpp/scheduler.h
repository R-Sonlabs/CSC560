/*
 * scheduler.h
 *
 * Created: 3/13/2019 6:35:35 PM
 *  Author: grace
 * scheduler.h
 */
 
#ifndef SCHEDULER_H_
#define SCHEDULER_H_
 
#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>

///Up to this many tasks can be run, in addition to the idle task
#define MAXTASKS  8

/**
 * Initialise the scheduler.  This should be called once in the setup routine.
 */
void Scheduler_Init();
 
/**
 * Go through the task list and run any tasks that need to be run.  The main function should simply be this
 * function called as often as possible, plus any low-priority code that you want to run sporadically.
 */
uint32_t Scheduler_Dispatch();


/************************************************************************/
/* Linked List Structure
 * Use template type to make a generic function LinkedList */
/************************************************************************/


template <class t> 
class LinkedList
{
	public:
		//Constructor
		LinkedList ()
		{
			head = NULL;
			tail = NULL;
			size = 0;
		}
		struct node
		{
			t val;
			node* next;
			node* prev;
			node(int v_) : val(v_), next(NULL) {};
		};
		
		t* front()
		{
			return &(head->val);
		}

		void addFront(t v_)
		{
			struct node* temp = (struct node*) malloc(sizeof(struct node)); 
			temp->val = v_;
			temp->next = head;
			head = temp;
			
		}
		void removeFront()
		{
			//struct Node* temp = *head_ref, *prev;
			node* temp = head;
			head = head->next;		
			free(temp);  // Free memory		
		}
		
		bool isEmpty()
		{
			return size == 0;
			
		}
		int length()
		{
			return size;
		}
		// Destructor
		~LinkedList(){
			
		}
		private:
		node* head;
		node* tail;
		int size;
			
};

struct task_arg{
	uint8_t pin;
	char port;	
};

//A task callback function
typedef void (*task_cb)(LinkedList<task_arg>&);


/**
 * Start a task.
 * The function "task" will be called roughly every "period" milliseconds starting after "delay" milliseconds.
 * The scheduler does not guarantee that the task will run as soon as it can.  Tasks are executed until completion.
 * If a task misses its scheduled execution time then it simply executes as soon as possible.  Don't pass stupid
 * values (e.g. negatives) to the parameters.
 *
 * \param id The tasks ID number.  This must be between 0 and MAXTASKS (it is used as an array index).
 * \param delay The task will start after this many milliseconds.
 * \param period The task will repeat every "period" milliseconds.
 * \param task The callback function that the scheduler is to call.
 */
void Scheduler_StartTask(int16_t delay, int16_t period, task_cb task, LinkedList<task_arg> argument);

void Scheduler_StartTask_Oneshot(task_cb task_oneshot, LinkedList<task_arg> argument, int priority, int32_t run_time, int32_t remaining_time);
void Scheduler_Dispatch_Oneshot();

#endif /* SCHEDULER_H_ */