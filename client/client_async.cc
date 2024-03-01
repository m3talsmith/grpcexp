#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpc/support/log.h>
#include <grpc++/grpc++.h>

#include "numbers.grpc.pb.h"

ABSL_FLAG(std::string, host, "localhost:3000", "GRPC host address");
ABSL_FLAG(int32_t, die, 20, "Dice to roll");

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
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
            CompletionQueue cq;

            Status status;
            
            std::unique_ptr<ClientAsyncResponseReader<GetRandomReply>> rpc(
                stub_->AsyncGetRandom(&context, request, &cq)
            );

            rpc->Finish(&reply, &status, (void*)1);

            void* got_tag;
            bool ok = false;

            // TODO: Assertions should be replaced with real error handling
            GPR_ASSERT(cq.Next(&got_tag, &ok));
            GPR_ASSERT(got_tag == (void*)1);
            GPR_ASSERT(ok);

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

int main(int argc, char* argv[]) {
    absl::ParseCommandLine(argc, argv);
    std::string host = absl::GetFlag(FLAGS_host);
    int32_t die = absl::GetFlag(FLAGS_die);

    NumbersClient client(
        grpc::CreateChannel(host, grpc::InsecureChannelCredentials())
    );
    
    int32_t number = client.GetRandom(die);
    if (number < 0)
    {
        std::cout << "An error occurred: see above output." << std::endl;
        return 1;
    }
    

    std::cout << "Your d" << die << " result is: " << number << std::endl;

    return 0;
}