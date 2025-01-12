#include <scipi2.h>

namespace scipi
{
scipi2::~scipi2()
{
}

void scipi2::update(){
    while (serial.available())
    {
        char c = serial.read();
        if(c=='\n' || c=='\r' || c==';'){
            input[inBufIndex]='\0';
            if(inBufIndex!=0)cmd.parse(serial, input);
            inBufIndex=0;
        }else if(c=='\b'){
            if(inBufIndex>0) inBufIndex--;
        }else{
            input[inBufIndex] = c;
            inBufIndex++;
        }
        if(inBufIndex>SCIPI_BUFFER_SIZE){
            inBufIndex=0;
            serial.write("Command too long\n");
            //read all available data to empty buffer
            while (serial.available())
            {
                serial.read();
            }
        }
    }
}

int Command::cmdInd(char* command, int c){
    for(int i=0;i<childCMDCount;i++){
        if(strncasecmp(subcommands[i].txt, command, c)==0){
            return i;
        }
    }
    return -1;
}

void Command::parse(HardwareSerial& serial, char* line){
    for(int i=0;line[i]!='\0';i++){
        //sub cmd
        if(line[i]==':'){
            int ind = cmdInd(line, i);
            if(ind==-1){
                serial.write("Command \"");
                serial.write(line, i);
                serial.write("\" not found.\n");
                return;
            }
            subcommands[ind].cmd->parse(serial, line+i+1);
            return;
        }
        else if(line[i]==' '){
            int ind = cmdInd(line, i);
            if(ind==-1){
                serial.write("Command \"");
                serial.write(line, i);
                serial.write("\" not found.\n");
                return;
            }
            char* argv[SCIPI_MAX_ARGC];
            int argc=1;
            line[i]='\0';
            argv[0]=line;
            i++;
            bool start=false;
            argv[argc]=line+i;

            do
            {
                if(!start){
                    if(line[i]!=' ')start=true;
                }else{
                    if(line[i]==' ' || line[i]=='\0'){
                        line[i]='\0';
                        argc++;
                        start=false;
                        if(argc==SCIPI_MAX_ARGC){
                            serial.write("Too many arguments.\n");
                            return;
                        }
                        argv[argc]=line+i+1;
                    }
                }
                i++;
            }while (line[i]!='\0');
            argc++;
            if(subcommands[ind].cmd->func==nullptr){
                serial.write("Call cmd not supported. Did you mean to add '?'?\n");
                return;
            }
            subcommands[ind].cmd->func(argc, argv);
            return;
        }else if(line[i]=='?'){
            int ind = cmdInd(line, i);
            if(ind==-1){
                serial.write("Command \"");
                serial.write(line);
                serial.write("\" not found.\n");
                return;
            }
            subcommands[ind].cmd->get();
            return;
        }
    }
    //call cmd
    int ind = cmdInd(line, strlen(line));
    if(ind==-1){
        serial.write("Command \"");
        serial.write(line);
        serial.write("\" not found.\n");
        return;
    }
    subcommands[ind].cmd->func(1, &line);
    return;
}

} // namespace scipi
