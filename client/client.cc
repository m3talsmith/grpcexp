#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "numbers.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using numbers::Numbers;
using numbers::GetRandomReply;
using numbers::GetRandomRequest;

class NumbersClient {
    public:
        NumbersClient(std::shared_ptr<Channel> channel) : stub_(Numbers::NewStub(channel)) {}
        
        int32_t GetRandom(const int32_t& number) {
            GetRandomRequest request;
            request.set_highest(number);

            GetRandomReply reply;
            ClientContext context;

            Status status = stub_->GetRandom(&context, request, &reply);

            if (status.ok()) {
                return reply.result();
            } else {
                std::cout << status.error_code() << ": "
                    << status.error_message() 
                    << std::endl;
                return -1;
            }
        }
    private:
        std::unique_ptr<Numbers::Stub> stub_;
};

int main(int argc, char** argv) {
    std::string server("localhost:3000");
    NumbersClient client(
        grpc::CreateChannel(server, grpc::InsecureChannelCredentials())
    );
    int32_t dice(20);
    int32_t number = client.GetRandom(dice);
    std::cout << "Your d" << dice << " result is: " << number << std::endl;

    return 0;
}