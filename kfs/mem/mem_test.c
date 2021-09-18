#include <kfs/kernel.h>
#include <kfs/mem.h>
#include <kfs/pages.h>
#include <string.h>

extern void		kmalloc_test_simple()
{
	printk(KERN_NOTICE "kmalloc_test_simple\n");
	char *test = (char *)kmalloc(sizeof(char) * 20);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	printk("%s", test);
	kfree(test);
	test = (char *)kmalloc(sizeof(char) * 500);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	printk("%s", test);
	kfree(test);
	test = (char *)kmalloc(sizeof(char) * 1000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	printk("%s", test);
	kfree(test);
}

extern void		kmalloc_test_large(void)
{
	printk(KERN_NOTICE "kmalloc_test_large\n");
	char		*test = (char *)kmalloc(sizeof(char) * 20000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	printk("space allocated = %d Bytes\n", kmalloc_get_size(test));
	printk("%s", test);
	strcpy((char *)((uint32_t)test + 2000), "test\n");
	printk("%s", (char *)((uint32_t)test + 2000));
	kfree(test);
	
	test = (char *)kmalloc(sizeof(char) * 20000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	printk("%s", test);
	strcpy((char *)((uint32_t)test + 4000), "test\n");
	printk("%s", (char *)((uint32_t)test + 4000));


	char		*test2 = (char *)kmalloc(sizeof(char) * 20000);
	if (test2 == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test2, "Hello World !\n");
	printk("%#x\n", (uint32_t)test2);
	printk("%s", test2);
	strcpy((char *)((uint32_t)test2 + 1000), "test\n");
	printk("%s", (char *)((uint32_t)test2 + 1000));
}

extern void		kmalloc_test_no_leak_small()
{
	char	*test;

	printk(KERN_NOTICE "kmalloc_test_no_leak_small\n");
	test = (char *)kmalloc(sizeof(char) * 20);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	kfree(test);

	for (int i = 0; i < 10000; i++) {
		test = (char *)kmalloc(sizeof(char) * 20);
		if (test == NULL) {
			panic("malloc failed\n");
		}
		strcpy(test, "Hello World !\n");
		kfree(test);
	}

	test = (char *)kmalloc(sizeof(char) * 20);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n\n", (uint32_t)test);
	kfree(test);
}

extern void		kmalloc_test_leak_small()
{
	char	*test;

	printk(KERN_NOTICE "kmalloc_test_leak_small\n");
	test = (char *)kmalloc(sizeof(char) * 20);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	kfree(test);

	for (int i = 0; i < 10000; i++) {
		test = (char *)kmalloc(sizeof(char) * 20);
		if (test == NULL) {
			panic("malloc failed\n");
		}
		strcpy(test, "Hello World !\n");
	//	kfree(test);
	}

	test = (char *)kmalloc(sizeof(char) * 20);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n\n", (uint32_t)test);
	kfree(test);
}

extern void		kmalloc_test_no_leak_medium()
{
	char	*test;

	printk(KERN_NOTICE "kmalloc_test_no_leak_medium\n");
	test = (char *)kmalloc(sizeof(char) * 500);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	kfree(test);

	for (int i = 0; i < 10000; i++) {
		test = (char *)kmalloc(sizeof(char) * 500);
		if (test == NULL) {
			panic("malloc failed\n");
		}
		strcpy(test, "Hello World !\n");
		kfree(test);
	}

	test = (char *)kmalloc(sizeof(char) * 500);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n\n", (uint32_t)test);
	kfree(test);
}

extern void		kmalloc_test_leak_medium()
{
	char	*test;

	printk(KERN_NOTICE "kmalloc_test_leak_medium\n");
	test = (char *)kmalloc(sizeof(char) * 500);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	kfree(test);

	for (int i = 0; i < 10000; i++) {
		test = (char *)kmalloc(sizeof(char) * 500);
		if (test == NULL) {
			panic("malloc failed\n");
		}
		strcpy(test, "Hello World !\n");
	//	kfree(test);
	}

	test = (char *)kmalloc(sizeof(char) * 500);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n\n", (uint32_t)test);
	kfree(test);
}


extern void		kmalloc_test_no_leak_large()
{
	char	*test;

	printk(KERN_NOTICE "kmalloc_test_no_leak_large\n");
	test = (char *)kmalloc(sizeof(char) * 1000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	kfree(test);

	for (int i = 0; i < 10000; i++) {
		test = (char *)kmalloc(sizeof(char) * 1000);
		if (test == NULL) {
			panic("malloc failed\n");
		}
		strcpy(test, "Hello World !\n");
		kfree(test);
	}

	test = (char *)kmalloc(sizeof(char) * 1000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n\n", (uint32_t)test);
	kfree(test);
}

extern void		kmalloc_test_leak_large()
{
	char	*test;

	printk(KERN_NOTICE "kmalloc_test_leak_large\n");
	test = (char *)kmalloc(sizeof(char) * 1000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n", (uint32_t)test);
	kfree(test);

	for (int i = 0; i < 10000; i++) {
		test = (char *)kmalloc(sizeof(char) * 1000);
		if (test == NULL) {
			panic("malloc failed\n");
		}
		strcpy(test, "Hello World !\n");
	//	kfree(test);
	}

	test = (char *)kmalloc(sizeof(char) * 1000);
	if (test == NULL) {
		panic("malloc failed\n");
	}
	strcpy(test, "Hello World !\n");
	printk("%#x\n\n", (uint32_t)test);
	kfree(test);
}