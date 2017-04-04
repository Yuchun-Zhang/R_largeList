#include "large_list.h"
namespace large_list {
	progressReporter::progressReporter() {
		clock_begin_ = clock();
		estimated_sec_times_ = 1000;
		is_long_time_ = FALSE;
	}

	void progressReporter::reportProgress(int i, int length, std::string progress_name) {
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

	void progressReporter::reportFinish(std::string progress_name) {
		clock_end_ = clock();
		if (is_long_time_ == TRUE) {
			Rprintf("\r                                    ");
			Rprintf("\r%s Finished in %.2f Seconds!\n", 
					progress_name.c_str(), 
					(double)(clock_end_ - clock_begin_) / CLOCKS_PER_SEC);
		}
		return;
	}
}