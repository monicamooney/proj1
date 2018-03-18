/*
 * main.h
 *
 *  Created on: Mar 15, 2018
 *      Author: Monica
 */

#ifndef DATASTRUCT_H_
#define DATASTRUCT_H_

/*typedef struct{
	char filename[128];
	int sequence_num;
	int count;
	char message[1024];
}PACKET;*/

typedef struct{
	uint16_t seq_num;
	uint16_t count;
}header, *h_pointer;


typedef struct{
	//uint128_t message;
}message, *m_pointer;

typedef struct{
	h_pointer;
	m_pointer;
}packet;

#endif /* DATASTRUCT_H_ */
