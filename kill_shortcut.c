#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/pid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Justino, Salomao Alves, Tarcisio Junio");
MODULE_DESCRIPTION("Mata um processo com uma combinacao de teclas");

//Índices do vetor de teclas
#define V_CTRL 0
#define V_ALT 1
#define V_SPACE 2

//Definição dos scancodes
#define CTRL   0x1D
#define ALT    0x38
#define SPACE  0x39

#define KEY_1  0x02
#define KEY_2  0x03
#define KEY_3  0x04
#define KEY_4  0x05
#define KEY_5  0x06
#define KEY_6  0x07
#define KEY_7  0x08
#define KEY_8  0x09
#define KEY_9  0x0A
#define KEY_0  0x0B

#define SCAN   0x7F
#define STATUS 0x80

#define SEG 0x1F
#define MIN 0x32
#define HOR 0x23

//static struct timer_list my_timer;

//Teclas pressionadas
int key_codes[3] = {0, 0, 0};
int pid = 0;

struct task_struct *processo;

unsigned char setting_time = 0;
unsigned char setting_pid = 0;

irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
	static unsigned char scancode;
	static unsigned char status;

	scancode = inb(0x60);
	status = inb(0x64);

	if((scancode & SCAN) == CTRL && (scancode & STATUS) == 0)
		key_codes[V_CTRL] = 1;
	if((scancode & SCAN) == CTRL && (scancode & STATUS) != 0)
		key_codes[V_CTRL] = 0;

	if((scancode & SCAN) == ALT && (scancode & STATUS) == 0)
		key_codes[V_ALT] = 1;
	if((scancode & SCAN) == ALT && (scancode & STATUS) != 0)
		key_codes[V_ALT] = 0;

	if((scancode & SCAN) == SPACE && (scancode & STATUS) == 0)
		key_codes[V_SPACE] = 1;
	if((scancode & SCAN) == SPACE && (scancode & STATUS) != 0)
		key_codes[V_SPACE] = 0;

	//Lê o pid
	if (setting_pid && !setting_time && !(scancode & STATUS)) {
		switch(scancode & SCAN) {
			case KEY_1:
				pid *= 10;
				pid += 1;
			break;

			case KEY_2:
				pid *= 10;
				pid += 2;
			break;

			case KEY_3:
				pid *= 10;
				pid += 3;
			break;

			case KEY_4:
				pid *= 10;
				pid += 4;
			break;

			case KEY_5:
				pid *= 10;
				pid += 5;
			break;

			case KEY_6:
				pid *= 10;
				pid += 6;
			break;

			case KEY_7:
				pid *= 10;
				pid += 7;
			break;

			case KEY_8:
				pid *= 10;
				pid += 8;
			break;

			case KEY_9:
				pid *= 10;
				pid += 9;
			break;

			case KEY_0:
				pid *= 10;
			break;

			default:
			break;
				pid = 0;
		}
	}

	//LÊ o tempo
	if(!setting_pid && setting_time && !(scancode & STATUS)) {
		switch(scancode & SCAN){

		}
	}

	if(!setting_pid && !setting_time && key_codes[V_CTRL] && key_codes[V_ALT]){
		setting_pid = 1;
	}

	if(setting_pid && !setting_time && key_codes[V_CTRL] && key_codes[V_ALT] && key_codes[V_SPACE] && !(scancode & SCAN)) {
		setting_pid = 0;
		setting_time = 1;
	}


	//Finalizando o processo sem tempo programado
	if(setting_pid && !key_codes[V_CTRL] && !key_codes[V_ALT] && pid) {
		processo = pid_task(find_vpid(pid), PIDTYPE_PID);

		printk(KERN_INFO "Matando processo de PID: %d\n", pid);
		force_sig(9, processo);
		setting_pid = 0;
		pid = 0;
	}

	return (irq_handler_t) IRQ_HANDLED;
}

static int __init hello_init(void) {
	request_irq(1, (irq_handler_t) irq_handler, IRQF_SHARED, "devDriver_keyboard_irq", (void*) (irq_handler)); 
	printk(KERN_INFO "Atalho de matar o processo registrado\n");
	return 0;
}

static void __exit hello_exit(void) {
	free_irq(1, (void*) (irq_handler));
	printk(KERN_INFO "Registro removido do atalho de matar um processo\n");
}

module_init(hello_init);
module_exit(hello_exit);
