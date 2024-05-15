#include <iostream>
#include "src/mail.h"

int main(){
    try {
        std::cout<<"Send mail...\n";
        //sendMail(recipent_address, subject, content, "../credentials.json");

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}