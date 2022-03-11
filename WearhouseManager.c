#include <stdio.h>
#include "WearhouseManager.h"


Package *create_package(long priority, const char* destination){
	Package *package=(Package*)malloc(sizeof(Package));
	if(package==NULL) return NULL;
	package->destination=(char*)malloc(100*sizeof(char));
	package->priority=priority;
	if(destination==NULL)
		package->destination=NULL;
	else strcpy(package->destination,destination);	
	return package;
}

void destroy_package(Package* package){
	if(package==NULL) return;
	free(package->destination);
	free(package);
}

Manifest* create_manifest_node(void){
	Manifest* manifest=(Manifest*)malloc(sizeof(Manifest));
	if(manifest==NULL) return NULL;
	manifest->next=NULL;
	manifest->prev=NULL;
	manifest->package=NULL;
	return manifest;
}

void destroy_manifest_node(Manifest* manifest_node){
	if(manifest_node==NULL) return;
	Manifest *aux,*p;
	p=manifest_node;
	while(p!=NULL){
		aux=p;
		p=p->next;
		free(aux);
	}
	free(manifest_node);
	manifest_node=NULL;
}

Wearhouse* create_wearhouse(long capacity){
	if(capacity==0) return NULL;
	Wearhouse *wearhouse=(Wearhouse*)malloc(sizeof(Wearhouse));
	wearhouse->capacity=capacity;
	wearhouse->size=0;
	wearhouse->packages=(Package**)malloc(capacity*sizeof(Package*));
	return wearhouse;
}

Wearhouse *open_wearhouse(const char* file_path){
	ssize_t read_size;
	char* line = NULL;
	size_t len = 0;
	char* token = NULL;
	Wearhouse *w = NULL;


	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		goto file_open_exception;

	if((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		w = create_wearhouse(atol(token));

		free(line);
		line = NULL;
		len = 0;
	}

	while((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		long priority = atol(token);
		token = strtok(NULL, ",\n ");
		Package *p = create_package(priority, token);
		w->packages[w->size++] = p;

		free(line);
		line = NULL;
		len = 0;
	}

	free(line);


	fclose(fp);
	return w;

	file_open_exception:
	return NULL;
}

long wearhouse_is_empty(Wearhouse *w){
	if(w->size==0) return 1;
	return 0;
}

long wearhouse_is_full(Wearhouse *w){
	if(w->size>=w->capacity) return 1;
	return 0;
}

long wearhouse_max_package_priority(Wearhouse *w){
	long max;
	max=w->packages[0]->priority;
	for(int i=0;i<w->size;i++){
		if(max < w->packages[i]->priority){
			max=w->packages[i]->priority;
		}
	}
	return max;
}

long wearhouse_min_package_priority(Wearhouse *w){
	long min;
	min=w->packages[0]->priority;
	for(int i=0;i<w->size;i++){
		if(min > w->packages[i]->priority){
			min=w->packages[i]->priority;
		}
	}
	return min;
}


void wearhouse_print_packages_info(Wearhouse *w){
	for(long i = 0; i < w->size; i++){
		printf("P: %ld %s\n",
				w->packages[i]->priority,
				w->packages[i]->destination);
	}
	printf("\n");
}

void destroy_wearhouse(Wearhouse* wearhouse){
	if(wearhouse==NULL) return;
	for(int i=0;i<wearhouse->size;i++){
		free(wearhouse->packages[i]);
	}
	free(wearhouse);
}


Robot* create_robot(long capacity){
	Robot* robot=(Robot*)malloc(sizeof(Robot));
	if(robot==NULL) return NULL;
	robot->size=0;
	robot->capacity=capacity;
	robot->manifest=NULL;
	robot->next=NULL;
	return robot;
}

int robot_is_full(Robot* robot){
	if(robot->size==robot->capacity) return 1;
	return 0;
}

int robot_is_empty(Robot* robot){
	if(robot->size==0) return 1;
	return 0;
}

Package* robot_get_wearhouse_priority_package(Wearhouse *w, long priority){
	for(long i=0;i<w->size;i++){
		if(w->packages[i]->priority==priority)
			return w->packages[i];
	}
	return NULL;
}

void robot_remove_wearhouse_package(Wearhouse *w, Package* package){	
	long i;	
	for(i=0;i<w->size;i++){
		if(w->packages[i]==package){
			break;
		}
	}
	w->size=w->size-1;
	for(long j=i;j<w->size;j++)
		w->packages[j]=w->packages[j+1];
}

void robot_load_one_package(Robot* robot, Package* package){
	Manifest *newelem=create_manifest_node();	
	newelem->package=package;
	Manifest *current=robot->manifest;
	Manifest *aux=NULL;
	if(robot==NULL) return;
	if(robot->size==robot->capacity)  return;
	if(robot->size==0){
		robot->manifest=newelem;
		robot->size=robot->size+1;
		return;
	}
	while(current!=NULL){  
		if(package->priority>current->package->priority)
			break;
		if(current->package->priority==package->priority)
			if(package->destination[0]<current->package->destination[0])
				break;
		aux=current;		
		current=current->next;
	}
	if(current==NULL){
		newelem->next=NULL;
		newelem->prev=aux;
		aux->next=newelem;
		robot->size=robot->size+1;
		return;
	}
	if(current!=NULL && current->prev!=NULL){
		newelem->next=current;
		newelem->prev=current->prev;
		current->prev->next=newelem;
		current->prev=newelem;
		robot->size=robot->size+1;
		return;
	}
	if(current==robot->manifest){
			newelem->next=current;
			current->prev=newelem;
			robot->manifest=newelem;
			robot->size=robot->size+1;
			return;
	}
}

long robot_load_packages(Wearhouse* wearhouse, Robot* robot){
	long k=0;
	while(robot_is_full(robot)==0 && wearhouse->size>0){
robot_load_one_package(robot,robot_get_wearhouse_priority_package(wearhouse,wearhouse_max_package_priority(wearhouse)));
robot_remove_wearhouse_package(wearhouse,robot_get_wearhouse_priority_package(wearhouse,wearhouse_max_package_priority(wearhouse)));
		k++;
	}
	return k;
}

Package* robot_get_destination_highest_priority_package(Robot* robot, const char* destination){
	Manifest* current=robot->manifest;
	while(strcmp(current->package->destination,destination)!=0){
		current=current->next;
	}
	return current->package;
}

void destroy_robot(Robot* robot){
	destroy_manifest_node(robot->manifest);
	free(robot);
}

void robot_unload_packages(Truck* truck, Robot* robot){
	Manifest *current=robot->manifest;
	current->prev=NULL;
	while(current!=NULL && robot->size!=0 && truck->size!=truck->capacity){
		if(strcmp(truck->destination,current->package->destination)==0){
			Manifest *newelem=create_manifest_node();
			newelem->package=current->package;
			if(truck->size!=0){
				newelem->next=truck->manifest;
				truck->manifest->prev=newelem;
				truck->manifest=newelem;
				truck->size=truck->size+1;
			}
			else{
				truck->manifest=newelem;
				truck->size=1;
			}
			if(current->prev==NULL){
				robot->manifest=current->next;
				robot->size=robot->size-1;
			}
			else{
				if(current->next==NULL){
					current->prev->next=NULL;
					robot->size=robot->size-1;
				}
				else{
					current->next->prev=current->prev;
					current->prev->next=current->next;
					robot->size=robot->size-1;
				}
			}
		}
		current=current->next;
	}
	
}



// Attach to specific truck
int robot_attach_find_truck(Robot* robot, Parkinglot *parkinglot){
	int found_truck = 0;
	long size = 0;
	Truck *arrived_iterator = parkinglot->arrived_trucks->next;
	Manifest* m_iterator = robot->manifest;


	while(m_iterator != NULL){
		while(arrived_iterator != parkinglot->arrived_trucks){
			size  = truck_destination_robots_unloading_size(arrived_iterator);
			if(strncmp(m_iterator->package->destination, arrived_iterator->destination, MAX_DESTINATION_NAME_LEN) == 0 &&
					size < (arrived_iterator->capacity-arrived_iterator->size)){
				found_truck = 1;
				break;
			}

			arrived_iterator = arrived_iterator->next;
		}

		if(found_truck)
			break;
		m_iterator = m_iterator->next;
	}

	if(found_truck == 0)
		return 0;


	Robot* prevr_iterator = NULL;
	Robot* r_iterator = arrived_iterator->unloading_robots;
	while(r_iterator != NULL){
		Package *pkg = robot_get_destination_highest_priority_package(r_iterator, m_iterator->package->destination);
		if(m_iterator->package->priority >= pkg->priority)
			break;
		prevr_iterator = r_iterator;
		r_iterator = r_iterator->next;
	}

	robot->next = r_iterator;
	if(prevr_iterator == NULL)
		arrived_iterator->unloading_robots = robot;
	else
		prevr_iterator->next = robot;

	return 1;
}

void robot_print_manifest_info(Robot* robot){
	Manifest *iterator = robot->manifest;
	while(iterator != NULL){
		printf(" R->P: %s %ld\n", iterator->package->destination, iterator->package->priority);
		iterator = iterator->next;
	}

	printf("\n");
}



Truck* create_truck(const char* destination, long capacity, long transit_time, long departure_time){
	Truck *truck=(Truck*)malloc(sizeof(Truck));
	truck->destination=(char*)malloc(100*sizeof(char));
	if(destination==NULL)
		truck->destination=NULL;
	else
		strcpy(truck->destination,destination);
	truck->unloading_robots=NULL;
	truck->manifest=NULL;
	truck->size=0;
	truck->capacity=capacity;
	truck->in_transit_time=0;
	truck->transit_end_time=transit_time;
	truck->departure_time=departure_time;
	return truck;
}

int truck_is_full(Truck *truck){
	if(truck->size==truck->capacity) return 1;
	return 0;
}

int truck_is_empty(Truck *truck){
	if(truck->size==0) return 1;
 	return 0;
}

long truck_destination_robots_unloading_size(Truck* truck){
	Robot *current;
	current=truck->unloading_robots;
	while(current!=NULL){
		Manifest *aux;
		aux=truck->unloading_robots->manifest;
		while(aux!=NULL){
			if(strcmp(aux->package->destination,truck->destination)==0){
				return current->size;
			}
			aux=aux->next;
		}
	}
	return 0;
}


void truck_print_info(Truck* truck){
	printf("T: %s %ld %ld %ld %ld %ld\n", truck->destination, truck->size, truck->capacity,
			truck->in_transit_time, truck->transit_end_time, truck->departure_time);

	Manifest* m_iterator = truck->manifest;
	while(m_iterator != NULL){
		printf(" T->P: %s %ld\n", m_iterator->package->destination, m_iterator->package->priority);
		m_iterator = m_iterator->next;
	}

	Robot* r_iterator = truck->unloading_robots;
	while(r_iterator != NULL){
		printf(" T->R: %ld %ld\n", r_iterator->size, r_iterator->capacity);
		robot_print_manifest_info(r_iterator);
		r_iterator = r_iterator->next;
	}
}


void destroy_truck(Truck* truck){
	destroy_manifest_node(truck->manifest);
	Robot *current;
	current=truck->unloading_robots;
	while(current!=NULL){
		free(current);
		current=current->next;
	}
	free(truck->destination);
	free(truck);
}


Parkinglot* create_parkinglot(void){
	Parkinglot* lot=(Parkinglot*)malloc(sizeof(Parkinglot));
	lot->arrived_trucks=create_truck(NULL,0,0,0);
	lot->arrived_trucks->next=lot->arrived_trucks;
	lot->departed_trucks=create_truck(NULL,0,0,0);
	lot->departed_trucks->next=lot->departed_trucks;
	lot->pending_robots=create_robot(0);
	lot->pending_robots->next=lot->pending_robots;
	lot->standby_robots=create_robot(0);
	lot->standby_robots->next=lot->standby_robots;
	return lot;
}

Parkinglot* open_parckinglot(const char* file_path){
	ssize_t read_size;
	char* line = NULL;
	size_t len = 0;
	char* token = NULL;
	Parkinglot *parkinglot = create_parkinglot();

	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		goto file_open_exception;

	while((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		// destination, capacitym transit_time, departure_time, arrived
		if(token[0] == 'T'){
			token = strtok(NULL, ",\n ");
			char *destination = token;

			token = strtok(NULL, ",\n ");
			long capacity = atol(token);

			token = strtok(NULL, ",\n ");
			long transit_time = atol(token);

			token = strtok(NULL, ",\n ");
			long departure_time = atol(token);

			token = strtok(NULL, ",\n ");
			int arrived = atoi(token);

			Truck *truck = create_truck(destination, capacity, transit_time, departure_time);

			if(arrived)
				truck_arrived(parkinglot, truck);
			else
				truck_departed(parkinglot, truck);

		}else if(token[0] == 'R'){
			token = strtok(NULL, ",\n ");
			long capacity = atol(token);

			Robot *robot = create_robot(capacity);
			parkinglot_add_robot(parkinglot, robot);

		}

		free(line);
		line = NULL;
		len = 0;
	}
	free(line);

	fclose(fp);
	return parkinglot;

	file_open_exception:
	return NULL;
}

void parkinglot_add_robot(Parkinglot* parkinglot, Robot *robot){
	if(parkinglot==NULL) return;
	Robot *newelem=(Robot*)malloc(sizeof(Robot));
	newelem=robot;
	if(robot->size!=0){
		Robot *current=parkinglot->pending_robots->next;
		Robot *prev=parkinglot->pending_robots;
		while(current!=parkinglot->pending_robots){
				if(current->size<robot->size)
					break;
				prev=current;
				current=current->next;
		}
		prev->next=newelem;
		newelem->next=current;
		return;
	}
	else{
		Robot *current2=parkinglot->standby_robots->next;
		Robot *prev2=parkinglot->standby_robots;
		while(current2!=parkinglot->standby_robots){
			if(current2->capacity<robot->capacity)
				break;
			prev2=current2;
			current2=current2->next;
		}
		prev2->next=newelem;
		newelem->next=current2;
		return;
	}
}

void parkinglot_remove_robot(Parkinglot *parkinglot, Robot* robot){
	if(parkinglot==NULL) return;
	if(robot->size!=0){
		if(parkinglot->pending_robots->next==parkinglot->pending_robots)
			return;
		Robot *current=parkinglot->pending_robots->next;
		Robot *prev=parkinglot->pending_robots;
		while(current!=parkinglot->pending_robots && current->size!=robot->size){
			prev=current;
			current=current->next;
		}
		prev->next=current->next;
		current->next=NULL;
		return;
	}
	else {
		Robot *current2=parkinglot->standby_robots->next;
		Robot *prev2=parkinglot->standby_robots;
		while(current2!=parkinglot->standby_robots && current2->capacity!=robot->capacity){
			prev2=current2;
			current2=current2->next;
		}
		prev2->next=current2->next;
		current2->next=NULL;
		return;
	}
}

int parckinglot_are_robots_peding(Parkinglot* parkinglot){
	if(parkinglot->pending_robots->next==parkinglot->pending_robots) return 0;
	return 1;
}

int parkinglot_are_arrived_trucks_empty(Parkinglot* parkinglot){
	Truck *truck=parkinglot->arrived_trucks->next;
	while(truck!=parkinglot->arrived_trucks){
		if(truck->size==0)
			truck=truck->next;
		else{
			return 0;
		}
	}
	return 1;
}


int parkinglot_are_trucks_in_transit(Parkinglot* parkinglot){
	if(parkinglot->departed_trucks->next==parkinglot->departed_trucks) return 0;
	return 1;
}


void destroy_parkinglot(Parkinglot* parkinglot){
	destroy_truck(parkinglot->arrived_trucks);
	destroy_truck(parkinglot->departed_trucks);
	destroy_robot(parkinglot->pending_robots);
	destroy_robot(parkinglot->standby_robots);
	free(parkinglot);
}

void parkinglot_print_arrived_trucks(Parkinglot* parkinglot){
	Truck *iterator = parkinglot->arrived_trucks->next;
	while(iterator != parkinglot->arrived_trucks){

		truck_print_info(iterator);
		iterator = iterator->next;
	}

	printf("\n");

}

void parkinglot_print_departed_trucks(Parkinglot* parkinglot){
	Truck *iterator = parkinglot->departed_trucks->next;
	while(iterator != parkinglot->departed_trucks){
		truck_print_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");

}

void parkinglot_print_pending_robots(Parkinglot* parkinglot){
	Robot *iterator = parkinglot->pending_robots->next;
	while(iterator != parkinglot->pending_robots){
		printf("R: %ld %ld\n", iterator->size, iterator->capacity);
		robot_print_manifest_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");

}

void parkinglot_print_standby_robots(Parkinglot* parkinglot){
	Robot *iterator = parkinglot->standby_robots->next;
	while(iterator != parkinglot->standby_robots){
		printf("R: %ld %ld\n", iterator->size, iterator->capacity);
		robot_print_manifest_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");

}


void truck_departed(Parkinglot *parkinglot, Truck* truck){
	Truck *current=parkinglot->arrived_trucks->next;
	Truck *prev=parkinglot->arrived_trucks;
	while(current!=parkinglot->arrived_trucks){
		if(current->departure_time==truck->departure_time)
			break;
		prev=current;
		current=current->next;
	}
	if(current->departure_time==truck->departure_time){
		prev->next=current->next;
	}
	Truck *current2=parkinglot->departed_trucks->next;
	Truck *prev2=parkinglot->departed_trucks;
	while(current2!=parkinglot->departed_trucks){
		if(current2->departure_time>truck->departure_time)
			break;
		prev2=current2;
		current2=current2->next;
	}
	prev2->next=truck;
	truck->next=current2;
}

void truck_transfer_unloading_robots(Parkinglot* parkinglot, Truck* truck){
	Robot *current=truck->unloading_robots;
	Robot *prev;
	while(current!=NULL){
		prev=current;
		current=current->next;
		parkinglot_add_robot(parkinglot,prev);
	}
	truck->unloading_robots=NULL;

}

void robot_swarm_collect(Wearhouse *wearhouse, Parkinglot *parkinglot){
	Robot *head_robot = parkinglot->standby_robots;
	Robot *current_robot = parkinglot->standby_robots->next;
	while(current_robot != parkinglot->standby_robots){

		// Load packages from wearhouse if possible
		if(!robot_load_packages(wearhouse, current_robot)){
			break;
		}

		// Remove robot from standby list
		Robot *aux = current_robot;
		head_robot->next = current_robot->next;
		current_robot = current_robot->next;

		// Add robot to the
		parkinglot_add_robot(parkinglot, aux);
	}
}


void robot_swarm_assign_to_trucks(Parkinglot *parkinglot){

	Robot *current_robot = parkinglot->pending_robots->next;

	while(current_robot != parkinglot->pending_robots){
		Robot* aux = current_robot;
		current_robot = current_robot->next;
		parkinglot_remove_robot(parkinglot, aux);
		int attach_succeded = robot_attach_find_truck(aux, parkinglot);
		if(!attach_succeded)
			parkinglot_add_robot(parkinglot, aux);
	}
}

void robot_swarm_deposit(Parkinglot* parkinglot){
	Truck *arrived_iterator = parkinglot->arrived_trucks->next;
	while(arrived_iterator != parkinglot->arrived_trucks){
		Robot *current_robot = arrived_iterator->unloading_robots;
		while(current_robot != NULL){
			robot_unload_packages(arrived_iterator, current_robot);
			Robot *aux = current_robot;
			current_robot = current_robot->next;
			arrived_iterator->unloading_robots = current_robot;
			parkinglot_add_robot(parkinglot, aux);
		}
		arrived_iterator = arrived_iterator->next;
	}
}

