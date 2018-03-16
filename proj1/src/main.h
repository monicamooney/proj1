/*
 * main.h
 *
 *  Created on: Mar 15, 2018
 *      Author: Monica
 */

#ifndef MAIN_H_
#define MAIN_H_

typedef struct{
	char filename[128];
	int sequence_num;
	int count;
	char message[1024];
}packet, *p_packet;

#endif /* MAIN_H_ */
