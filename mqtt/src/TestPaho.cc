#include "TestPaho.hh"
       #include <unistd.h>

TestPaho::TestPaho(std::string name, std::string process, uint32_t instance) : PahoInterface(name,process,instance)
{
   this->registerCommands();
}
TestPaho::~TestPaho()
{
    this->Stop();
    this->Disconnect();
}
void TestPaho::registerCommands()
{
    this->addCommand("STATUS",std::bind(&TestPaho::status,this,std::placeholders::_1));
}
void TestPaho::status(web::json::value v)
{
    std::cout<<v<<std::endl;
    std::stringstream si;
    si << id() << "/STATUS";
    auto r = web::json::value::object();
    r["status"] = web::json::value::number(11);
    std::string sm = r.serialize();
    std::cout << "\nSending message..." << std::endl;
    mqtt::message_ptr pubmsg = mqtt::make_message(mqtt::string_ref(si.str().c_str()), mqtt::binary_ref(sm.c_str()));
    pubmsg->set_qos(0);
    _cli->publish(pubmsg)->wait_for(std::chrono::seconds(4));
        std::cout << "\nSend done message..." << std::endl;

}
void loop()
{
    // to be implemented
}

int main()
{
    TestPaho m("unessai","TestPaho",0);
    m.Connect("localhost",1883);
    m.Start();
    while (1)
    ::sleep(10);
}