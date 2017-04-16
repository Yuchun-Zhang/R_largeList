#include "large_list.h"
namespace large_list {
	ProgressReporter::ProgressReporter() {
		clock_begin_ = clock();
		estimated_sec_times_ = 1000;
		is_long_time_ = FALSE;
	}

	void ProgressReporter::reportProgress(int i, int length, std::string progress_name) {
		clock_t clock_current = clock();
		if (is_long_time_ == FALSE) {
			double one_loop_time = (double)(clock_current - clock_begin_) / CLOCKS_PER_SEC / (i+1);
			// Rprintf("one loop time %lf\n", one_loop_time);
			if (one_loop_time * length > 5) {
				is_long_time_ = TRUE;
				estimated_sec_times_ = (int)1/one_loop_time;
			}
		}
		if (is_long_time_ == TRUE) {
			if ((int)(i/estimated_sec_times_) != (int)((i+1)/estimated_sec_times_)) {
				Rprintf("\r                                    ");
				Rprintf("\r%s %2.2lf%% ", progress_name.c_str(), ((double)i*100/length));
				R_FlushConsole(); 
				R_CheckUserInterrupt();
			}
		}
		return;
	}

	void ProgressReporter::reportFinish(std::string progress_name) {
		clock_end_ = clock();
		if (is_long_time_ == TRUE) {
			Rprintf("\r                                    ");
			Rprintf("\r%s Finished in %.2f Seconds!\n", 
					progress_name.c_str(), 
					(double)(clock_end_ - clock_begin_) / CLOCKS_PER_SEC);
		}
		return;
	}

	MemorySlot::MemorySlot() {
		for(int i = 0; i < NUMBER_OF_MEM_SLOTS; i++) {
			is_slot_in_use[i] = FALSE;
			is_slot_initialized[i] = FALSE;
			slot_size[i] = FALSE;
		}
	}
	void* MemorySlot::slot_malloc(int64_t length) {
		int i = 0;
		for (i = 0; is_slot_in_use[i] == TRUE; i++) {};
		if (is_slot_initialized[i] == FALSE) {
			is_slot_initialized[i] = TRUE;
			slot_size[i] = length;
			slot[i] = (void*) std::malloc (slot_size[i]);
			// Rprintf("Initialize slot %d with length %.0lf\n", i, (double)slot_size[i]);
		} else {
			if (length > slot_size[i]) {
				std::free(slot[i]);
				slot_size[i] = slot_size[i]*2 > length ? slot_size[i]*2 : length;
				slot[i] = (void*) std::malloc (slot_size[i]);
				// Rprintf("Change slot %d to length %.0lf\n", i, (double)slot_size[i]);
			}
			// Rprintf("Alloc slot %d to length %.0lf\n", i, (double)slot_size[i]);
		}
		is_slot_in_use[i] = TRUE;
		return(slot[i]);
	}
	void MemorySlot::slot_free(void * char_pointer) {
		int i = 0;
		while (TRUE) {
			if (is_slot_initialized[i] && is_slot_in_use[i]) {
				if (char_pointer == slot[i]) {
					break;
				} 
			}
			i++;
		}
		is_slot_in_use[i] = FALSE;
		// Rprintf("Clear slot %d with length %.0lf\n", i, (double)slot_size[i]);
		return;
	}
	void * MemorySlot::slot_realloc(void * char_pointer, int64_t length) {
		int i = 0;
		while (TRUE) {
			if (is_slot_initialized[i] && is_slot_in_use[i]) {
				if (char_pointer == slot[i]) {
					break;
				} 
			}
			i++;
		}
		if (length <= slot_size[i]) {
			// Rprintf("Realloc slot %d to length %.0lf\n within slot\n", i, (double)slot_size[i]);
			return(char_pointer);
		} else {
			std::free (slot[i]);
			slot_size[i] = slot_size[i]*2 > length ? slot_size[i]*2 : length;
			slot[i] = (void*) std::malloc (slot_size[i]);
			// Rprintf("Realloc slot %d to length %.0lf\n", i, (double)slot_size[i]);
			return(slot[i]);
		}
	}

	MemorySlot::~MemorySlot() {
		for (int i = 0; i < NUMBER_OF_MEM_SLOTS; i++) {
			if (is_slot_initialized[i]) {
				std::free (slot[i]);
				// Rprintf("Free meomory slot %d\n", i);
			}
		}
	}
}