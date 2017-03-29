#ifndef NANOS6_TASK_INSTANTIATION_H
#define NANOS6_TASK_INSTANTIATION_H


#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

//! \brief Data type to express priorities
typedef signed long nanos_priority_t;


//! \brief Struct that contains the common parts that all tasks of the same type share
typedef struct
{
	//! \brief Wrapper around the actual task implementation
	//! 
	//! \param[in,out] args_block A pointer to a block of data for the parameters
	void (*run)(void *args_block);
	
	//! \brief Function that the runtime calls to retrieve the information needed to calculate the dependencies
	//! 
	//! This function should call the nanos_register_input_dep, nanos_register_output_dep and nanos_register_inout_dep
	//! functions to pass to the runtime the information needed to calculate the dependencies
	//! 
	//! \param[in] handler a handler to be passed on to the registration functions
	//! \param[in] args_block a pointer to a block of data for the parameters partially initialized
	void (*register_depinfo)(void *handler, void *args_block);
	
	//! \brief Function that the runtime calls to obtain a user-specified priority for the task instance
	//! 
	//! Note that this field can be null to indicate the default priority.
	//! 
	//! \param[in] args_block a pointer to a block of data for the parameters partially initialized
	//! \returns a value that represents the desired task priority
	nanos_priority_t (*get_priority)(void *args_block);
	
	//! \brief A string that identifies the type of task
	char const *task_label;
	
	//! \brief A string that identifies the source location of the definition of the task
	char const *declaration_source;
	
	//! \brief Function that the runtime calls to obtain an estimation of the cost of the task
	//! 
	//! \param[in] args_block a pointer to a block of data for the parameters partially initialized
	//! \returns a value that represents the cost of the task
	size_t (*get_cost)(void *args_block);
} nanos_task_info __attribute__((aligned(64)));


//! \brief Struct that contains data shared by all tasks invoked at fixed location in the source code
typedef struct
{
	//! \brief A string that identifies the source code location of the task invocation
	char const *invocation_source;
} nanos_task_invocation_info __attribute__((aligned(64)));


typedef enum {
	//! Specifies that the task will be a final task
	nanos_final_task = (1 << 0),
	//! Specifies that the task is in "if(0)" mode
	nanos_if_0_task = (1 << 1)
} nanos_task_flag;


//! \brief Allocate space for a task and its parameters
//! 
//! This function creates a task and allocates space for its parameters.
//! After calling it, the user code should fill out the block of data stored in args_block_pointer,
//! and call nanos_submit_task with the contents stored in task_pointer.
//! 
//! \param[in] task_info a pointer to the nanos_task_info structure
//! \param[in] task_invocation_info a pointer to the nanos_task_invocation_info structure
//! \param[in] args_block_size size needed to store the paramerters passed to the task call
//! \param[out] args_block_pointer a pointer to a location to store the pointer to the block of data that will contain the parameters of the task call
//! \param[out] task_pointer a pointer to a location to store the task handler
void nanos_create_task(
	nanos_task_info *task_info,
	nanos_task_invocation_info *task_invocation_info,
	size_t args_block_size,
	/* OUT */ void **args_block_pointer,
	/* OUT */ void **task_pointer,
	size_t flags
);


//! \brief Submit a task
//! 
//! This function should be called after filling out the block of parameters of the task. See nanos_create_task.
//! 
//! \param[in] task The task handler
void nanos_submit_task(void *task);


#ifdef __cplusplus
}
#endif


#endif /* NANOS6_TASK_INSTANTIATION_H */
