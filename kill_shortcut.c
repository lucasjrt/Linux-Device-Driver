#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>

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

#define S 0x1F
#define M 0x32
#define H 0x23

static struct timer_list my_timer;

//Teclas pressionadas
unsigned int key_codes[3] = {0, 0, 0};
unsigned int pid_aux = 0;
unsigned int pid = 0;
unsigned long int time_aux = 0;
unsigned long int time_aux_I = 0;
unsigned long int time = 0;


unsigned char setting_time = 0;
unsigned char setting_pid = 0;
unsigned char millisec = 0;
unsigned char timing = 0;

struct task_struct *processo;


void close_the_process(int pidR) {
	processo = pid_task(find_vpid(pidR), PIDTYPE_PID);
	force_sig(9, processo);
}

void my_timer_callback(unsigned long data) {
	close_the_process(pid);
	//printk(KERN_INFO "~Processo de pid %d é finalizado\n", pid);
	timing = 0;
}

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
				pid_aux *= 10;
				pid_aux += 1;
				break;
			case KEY_2:
				pid_aux *= 10;
				pid_aux += 2;
				break;
			case KEY_3:
				pid_aux *= 10;
				pid_aux += 3;
				break;
			case KEY_4:
				pid_aux *= 10;
				pid_aux += 4;
				break;
			case KEY_5:
				pid_aux *= 10;
				pid_aux += 5;
				break;
			case KEY_6:
				pid_aux *= 10;
				pid_aux += 6;
				break;
			case KEY_7:
				pid_aux *= 10;
				pid_aux += 7;
				break;
			case KEY_8:
				pid_aux *= 10;
				pid_aux += 8;
				break;
			case KEY_9:
				pid_aux *= 10;
				pid_aux += 9;
				break;
			case KEY_0:
				pid_aux *= 10;
				break;
		}
	}

	//Lê o tempo
	if(!setting_pid && setting_time && !(scancode & STATUS)) {
		switch(scancode & SCAN){
			case KEY_1:
				time_aux_I *= 10UL;
				time_aux_I += 1UL;
				break;
			case KEY_2:
				time_aux_I *= 10UL;
				time_aux_I += 2UL;
				break;
			case KEY_3:
				time_aux_I *= 10UL;
				time_aux_I += 3UL;
				break;
			case KEY_4:
				time_aux_I *= 10UL;
				time_aux_I += 4UL;
				break;
			case KEY_5:
				time_aux_I *= 10UL;
				time_aux_I += 5UL;
				break;
			case KEY_6:
				time_aux_I *= 10UL;
				time_aux_I += 6UL;
				break;
			case KEY_7:
				time_aux_I *= 10UL;
				time_aux_I += 7UL;
				break;
			case KEY_8:
				time_aux_I *= 10UL;
				time_aux_I += 8UL;
				break;
			case KEY_9:
				time_aux_I *= 10UL;
				time_aux_I += 9UL;
				break;
			case KEY_0:
				time_aux_I *= 10UL;
				break;
			case S:
				if(!millisec)
					millisec = 1;
				time_aux += time_aux_I * 1000UL;
				time_aux_I = 0UL;
				break;
			case M:
				if(!millisec)
					millisec = 1;
				time_aux += time_aux_I * 60000UL;
				time_aux_I = 0UL;
				break;
			case H:
				if(!millisec)
					millisec = 1;
				time_aux += time_aux_I * 3600000UL;
				time_aux_I = 0UL;
				break;
		}
	}

	//Ativa a leitura do PID
	if(!setting_pid && !setting_time && key_codes[V_CTRL] && key_codes[V_ALT]){
		setting_pid = 1;
	}

	//Ativa a leitura do timer
	if(setting_pid && !setting_time && key_codes[V_CTRL] && key_codes[V_ALT] && key_codes[V_SPACE]) {
		setting_pid = 0;
		setting_time = 1;
	}

	//Finalizando o processo instantâneo
	if(setting_pid  && !setting_time && !key_codes[V_CTRL] && !key_codes[V_ALT] && pid_aux) {
		pid = pid_aux;
		close_the_process(pid);
		printk(KERN_INFO "Matando processo de PID: %d\n", pid_aux);
		setting_pid = 0;
		setting_time = 0;
		time_aux = 0;
		pid_aux = 0;
	}

	//Finalizando o proceso com timer
	if(!setting_pid && setting_time && !key_codes[V_CTRL] && !key_codes[V_ALT] && pid_aux && time_aux && !timing) {
		unsigned long int h = 0, m = 0, s = 0;
		if(!millisec)
			time_aux *= 1000;
		pid = pid_aux;
		time = time_aux;
		time_aux /= 1000;
		//h = time_aux / 3600000;
		//m = time_aux / 60000;
		//s = time_aux % 1000;
		printk(KERN_INFO "Convertendo %lu para horas minutos e segundos\n", time_aux);
		if(time_aux > 3600) {
			m = time_aux / 60;
			s = time_aux % 60;
			h = m / 60;
			m = m % 60;
		} else {
			m = time_aux / 60;
			s = time_aux % 60;
		}
		setup_timer(&my_timer, my_timer_callback, 0);
		mod_timer(&my_timer, jiffies + msecs_to_jiffies(time));
		printk(KERN_INFO "Finalizando o processo de PID: %d em %luh%lum%lus\n", pid, h, m, s);
		timing = 1;
		setting_pid = 0;
		setting_time = 0;
		time_aux = 0UL;
		pid_aux = 0;
	}

	//Não permite definir um tempo
	if(!setting_pid && setting_time && !key_codes[V_CTRL] && !key_codes[V_ALT] && pid_aux && time_aux && timing) {
		printk(KERN_INFO "O processo de PID %d ja foi programado para ser fechado em %lus\n", pid_aux, millisec?time_aux/1000:time_aux);
	}

	if(!setting_pid && !setting_time && !key_codes[V_CTRL] && !key_codes[V_ALT] && (pid_aux || time_aux)) {
		pid_aux = 0;
		time_aux = 0;
	}

	return (irq_handler_t) IRQ_HANDLED;
}

static int __init hello_init(void) {
	// unsigned long int test = 100;
	// printk(KERN_INFO "Teste de ulitoa: %s\n", ulitoa(test));
	request_irq(1, (irq_handler_t) irq_handler, IRQF_SHARED, "devDriver_keyboard_irq", (void*) (irq_handler)); 
	printk(KERN_INFO "O módulo de atalho para finalizar um processo pelo pid foi registrado\n");
	return 0;
}

static void __exit hello_exit(void) {
	free_irq(1, (void*) (irq_handler));
	try_to_del_timer_sync(&my_timer);
	printk(KERN_INFO "O módulo de atalho para finalizar um processo pelo pid foi removido\n");
}

module_init(hello_init);
module_exit(hello_exit);



// void swap_char(char *a, char *b) {
// 	char *aux = a;
// 	a = b;
// 	b = aux;
// }

// char* reverse_a_string(char* str, int tam) {
// 	int i = 0;
// 	static char* aux;
// 	aux  = (char*)vmalloc(sizeof(char) * tam);
// 	while(str[i] != '\0') {
// 		aux[i] = str[tam - i - 1];
// 		i++;
// 	}
// 	aux[i] = '\0';
// 	return aux;
	
// }

// char* ulitoa(unsigned long int num) {	
// 	char *str;
// 	int i = 0;
// 	str = (char*) vmalloc(sizeof(char) * 19);

//     if (num == 0) {
//         str[i++] = '0';
//         str[i] = '\0';
//         return str;
//     }

//     while (num != 0) {
//         int mod = num % 10;
//         str[i++] = (mod > 9)? (mod -10) + 'a' : mod + '0';
//         num = num/10;
//     }
//     str[i] = '\0';

//     str = reverse_a_string(str, i);

//     return str;

// }

// char *print_time_programmed(unsigned long int time) {
// 	char *str;
// 	str = (char*) vmalloc(sizeof(char) * 19);
// 	if(!time / 1000UL)
//     	return "0s";
//     //if(!((int)time / 60000)) {
//     	//strcpy(str, ulitoa(time / 1000);
//     	return str;
//     //}

// 	//("%s%s%s", time/3600000?ulitoa(time % 3600000)"h":"", time/60000?ulitoa(time % 60000)"m":"");
// 	return str;
// }
