#include "splayer_ios_birdge_impl.hpp"

void* init_person(){
    return new Person();
}
    
void person_introduceMySelf(void *person){
    Person *p = (Person*)person;
    p->introduceMySelf();
}
