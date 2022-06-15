/**
 * @file tests.c
 * @author:
 * @modifier: Jiahao Wei, Zitai Kong, Guanshujie Fu
 * @description: functions for keyboard operation
 * @creat_date: 2022.3. -  add tests functions for cp1
 *             2022.3.19 - add page fault test
 *             2022.3.20 - add system call test
 */

#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "drivers/rtc.h"
#include "kernel/paging.h"
#include "drivers/keyboard.h"
#include "drivers/filesystem.h"
#include "drivers/terminal.h"

#define PASS 1
#define FAIL 0
#define CP1_TEST 0
#define CP2_RTC_TEST 0
#define CP2_TERMINAL_TEST 0
#define CP2_FILESYSTEM_TEST 0

#define BUF_SIZE 128

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)                      \
	printf("Press Enter to start [%s TEST].\n", name); \
	while (key_pressed != 0x1c)                        \
		;                                              \
	key_pressed = 0;                                   \
	clear();                                           \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure()
{
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test()
{
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i)
	{
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			assertion_failure();
			result = FAIL;
		}
	}

	printf("\n=============== System Call Test ===============\n");
	printf("Try to access INT 0x80\n");
	printf("Press Enter to test System Call or Right Shift to skip.\n");
	while (key_pressed != 0x1c && key_pressed != 0x36)
		;
	if (key_pressed == 0x1c)
		asm("int $0x80");
	key_pressed = 0;

	return result;
}

/**
 * brief: show keyboard input on screen
 *
 * @return int
 */
int keyboard()
{
	TEST_HEADER;

	printf("\nPress ` to exit test.\n");
	printf("Start Enter: ");
	while (key_pressed != '`')
		;
	key_pressed = 0;
	return PASS;
}

/* RTC Test
 *
 * Receiving an RTC interrupt
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Coverage: RTC handler
 * Files: rtc.h/c
 */
int rtc_test()
{
	TEST_HEADER;

	printf("Press Enter to test rtc.\n");
	while (key_pressed != 0x1c)
		;
	key_pressed = 0;
	turn_on_rtc();
	while (key_pressed != 0x1c)
		;
	key_pressed = 0;
	turn_off_rtc();

	return PASS;
}

/**
 * brief: 1. show the contents of page dir and table
 *        2. dereferencing different address ranges with paging turned on
 * inputs: none
 * outputs: print the contents to screen
 */
int paging_test(void)
{
	TEST_HEADER;

	uint32_t addr;
	uint32_t page_fault;

	show_page_dir(kernel_page_dir);
	show_page_table(kernel_page_table_0_4M);

	for (addr = 0x400000; addr < 0x800000; addr += 0x100000 + 42)
	{
		printf("Addr: %#x\tContent: %#x\n", addr, *(uint32_t *)addr);
	}
	addr = 0x8F0000;
	printf("\n=============== Page Fault Exception Test ===============\n");
	printf("Try to access address: %#x \n", addr);
	printf("Press Enter to test page fault or Right Shift to skip\n");
	while (key_pressed != 0x1c && key_pressed != 0x36)
		;
	if (key_pressed == 0x1c)
		page_fault = *(uint32_t *)addr;
	else
		page_fault = 0;
	key_pressed = 0;

	return PASS;
}

/* Checkpoint 2 tests */
/* RTC Test
 *
 * RTC interrupt with different rates
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Coverage: RTC handler
 * Files: rtc.h/c
 */
int rtc_rate_test()
{
	TEST_HEADER;
	int32_t rate = 2;
	int32_t i;

	for (i = 0; i < 10; i++)
	{
		if (i == 0)
		{
			printf("Press Enter to test 2Hz rtc.\n");
			while (key_pressed != 0x1c)
				;
			key_pressed = 0;
			clear();
			rtc_open();
			turn_on_rtc();
			while (key_pressed != 0x1c)
				;
			key_pressed = 0;
			turn_off_rtc();
			clear();
		}
		else
		{
			printf("Press Enter to test %dHz rtc.\n", rate);
			while (key_pressed != 0x1c)
				;
			key_pressed = 0;
			clear();
			rtc_write(NULL, &rate, 4);
			turn_on_rtc();
			while (key_pressed != 0x1c)
				;
			key_pressed = 0;
			turn_off_rtc();
			clear();
		}
		rate = rate * 2;
	}

	rtc_close();

	return PASS;
}


/* filesystem_test_1
 *
 * fs tests for read_dentry_by_name()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: filesystem.h/c
 */
int filesystem_test_1(void)
{
	TEST_HEADER;
	dentry_t my_dentry;
	int32_t i;
	int8_t *filenames[4] = {".", "..", "cat", "verylargetextwithverylongname.tx"};

	for (i = 0; i < 4; i++)
	{
		printf("Read dentry by name %s\n", filenames[i]);
		if (read_dentry_by_name((uint8_t *)filenames[i], &my_dentry) == 0)
		{
			show_dentry(&my_dentry);
			printf("    dentry found\n");
		}
		else
		{
			printf("    dentry not exist\n");
		}
	}
	return PASS;
}


/* filesystem_test_2
 *
 * fs tests for read_dentry_by_index()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: filesystem.h/c
 */
int filesystem_test_2(void)
{
	TEST_HEADER;
	int32_t i;
	int32_t indexes[4] = {0, 4, 7, 19};
	dentry_t my_dentry;
	for (i = 0; i < 4; i++)
	{
		printf("Read dentry by index %d\n", indexes[i]);
		if (read_dentry_by_index(indexes[i], &my_dentry) == 0)
		{
			show_dentry(&my_dentry);
			printf("    dentry found\n");
		}
		else
		{
			printf("    index not valid\n");
		}
	}
	return PASS;
}


/* filesystem_test_3
 *
 * fs tests for read_data()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: filesystem.h/c
 */
int filesystem_test_3(void)
{
	TEST_HEADER;
	uint8_t buf;
	uint32_t i;
	dentry_t my_dentry;
	read_dentry_by_name((uint8_t *)"frame0.txt", &my_dentry);
	// read through all the possible bytes
	for (i = 0; i < data_blk_count_max * blk_size; i++)
	{
		if (read_data(my_dentry.inode_num, i, &buf, 1) != 1)
			break;
		putc(buf);
	}
	show_dentry(&my_dentry);
	printf("Number of bytes read: %d\n", i);
	if (i == GET_FILE_SIZE((&my_dentry)))
		return PASS;
	else
		return FAIL;
}


/* filesystem_test_4
 *
 * fs tests for read_data()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: filesystem.h/c
 */
int filesystem_test_4(void)
{
	TEST_HEADER;
	uint8_t buf[blk_size*2];
	uint32_t num_bytes = blk_size*2;
	uint32_t num_bytes_read;
	uint32_t i,j;
	dentry_t my_dentry;
	read_dentry_by_name((uint8_t *)"verylargetextwithverylongname.tx", &my_dentry);
	// read through all the possible bytes
	for (i = 0; i < data_blk_count_max * blk_size; i += num_bytes_read)
	{
		num_bytes_read = read_data(my_dentry.inode_num, i, buf, num_bytes);
		for (j = 0; j < num_bytes_read; j++)
		{
			putc(buf[j]);
		}
		if(num_bytes_read == 0) break;
	}
	putc('\n');
	show_dentry(&my_dentry);
	printf("Number of bytes read: %d\n", i);
	if (i == GET_FILE_SIZE((&my_dentry)))
		return PASS;
	else
		return FAIL;
}

/* filesystem_test_5
 *
 * fs tests for read executable
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: filesystem.h/c
 */
int filesystem_test_5(void)
{
	TEST_HEADER;
	uint8_t buf[1000];
	uint32_t num_bytes = 1000;
	uint32_t num_bytes_read;
	uint32_t i,j;
	dentry_t my_dentry;
	read_dentry_by_name((uint8_t *)"ls", &my_dentry);
	// read through all the possible bytes
	for (i = 0; i < data_blk_count_max * blk_size; i += num_bytes_read)
	{
		num_bytes_read = read_data(my_dentry.inode_num, i, buf, num_bytes);
		for (j = 0; j < num_bytes_read; j++)
		{
			putc(buf[j]);
		}
		if(num_bytes_read == 0) break;
	}
	putc('\n');
	show_dentry(&my_dentry);
	printf("Number of bytes read: %d\n", i);
	if (i == GET_FILE_SIZE((&my_dentry)))
		return PASS;
	else
		return FAIL;
}


/* filesystem_test_6
 *
 * fs tests for dir_read()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: filesystem.h/c
 */
int filesystem_test_6(void)
{
	// TEST_HEADER;
	// int i;
	// dentry_t my_dentry;
	// for (i = 0; i < 17; i++) {
	// 	dir_read(i, &my_dentry, dentry_size);
	// 	show_dentry(&my_dentry);
	// }
	return PASS;
}


/* terminal_driver_read_test
 *
 * tests for terminal read()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: terminal.h/c
 */
int terminal_driver_read_test()
{
	// TEST_HEADER;
	// int result = PASS;
	// char buf[BUF_SIZE];
	// terminal_open();
	// while (1){
	// 	memset(buf, 0, BUF_SIZE);
	// 	terminal_read(0, buf, BUF_SIZE);
	// 	printf("%s", buf);
	// }
	// terminal_close();
	// if (result == PASS)	printf("Terminal read test success.\n");
	return 0;
}
/* terminal_driver_write_test
 *
 * tests for terminal write()
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Files: terminal.h/c
 */
int terminal_driver_write_test()
{
	// TEST_HEADER;
	// int result = PASS;
	// char buf[BUF_SIZE];
	// terminal_open();
	// while (1){
	// 	memset(buf, 0, BUF_SIZE);
	// 	int a = terminal_read(0, buf, BUF_SIZE);
	// 	terminal_write(0, buf, a);
	// }
	// terminal_close();
	// if (result == PASS)	printf("Terminal read/write test success.\n");
	return 0;
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */



/* Test suite entry point */
void launch_tests()
{
	// launch your tests here
	#if (CP1_TEST)
	{
		TEST_OUTPUT("rtc_test", rtc_test());
		TEST_OUTPUT("idt_test", idt_test());
		TEST_OUTPUT("paging_test", paging_test());
		TEST_OUTPUT("keyboard_test", keyboard());
	}
	#endif
	#if (CP2_RTC_TEST)
	{
		TEST_OUTPUT("rtc_rate_test", rtc_rate_test());
	}
	#endif
	#if (CP2_FILESYSTEM_TEST)
	{
		TEST_OUTPUT("filesystem_test_1", filesystem_test_1())
		TEST_OUTPUT("filesystem_test_2", filesystem_test_2());
		TEST_OUTPUT("filesystem_test_3", filesystem_test_3());
		TEST_OUTPUT("filesystem_test_4", filesystem_test_4());
		TEST_OUTPUT("filesystem_test_5", filesystem_test_5());
		TEST_OUTPUT("filesystem_test_6", filesystem_test_6());
	}
	#endif

	#if (CP2_TERMINAL_TEST)
	{
		TEST_OUTPUT("terminal_write_test", terminal_driver_write_test());
	}
	#endif
	printf("Test Ends!\n");
}
