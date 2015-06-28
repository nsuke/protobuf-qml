#include "hello.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/status.h>

#include <iostream>
#include <sstream>

class CallData {
public:
  virtual void process(bool ok) = 0;
};

class SayHelloCallData : public CallData {
public:
  SayHelloCallData(Hello::AsyncService* service,
                   grpc::ServerCompletionQueue* cq)
      : cq_(cq), service_(service), writer_(&context_) {
    service->RequestSayHello(&context_, &request_, &writer_, cq_, cq_, this);
  }

  void process(bool ok) final {
    if (done_) {
      std::cout << "SayHello: DONE" << std::endl;
      new SayHelloCallData(service_, cq_);
      delete this;
    } else {
      std::cout << "SayHello: PROCESS" << std::endl;
      response_.set_greet(std::string("Hello " + request_.name()));
      writer_.Finish(response_, grpc::Status::OK, this);
      done_ = true;
    }
  }

private:
  bool done_ = false;
  grpc::ServerContext context_;
  grpc::ServerCompletionQueue* cq_;
  Hello::AsyncService* service_;
  HelloRequest request_;
  grpc::ServerAsyncResponseWriter<HelloResponse> writer_;
  HelloResponse response_;
};

class BatchHelloCallData : public CallData {
public:
  BatchHelloCallData(Hello::AsyncService* service,
                     grpc::ServerCompletionQueue* cq)
      : cq_(cq), service_(service), reader_(&context_) {
    process(true);
  }

  void process(bool ok) final {
    if (status == Status::INIT) {
      ss << "Hello";
      service_->RequestBatchHello(&context_, &reader_, cq_, cq_, this);
      status = Status::CALL;
    } else if (status == Status::CALL) {
      std::cout << "BatchHello: CALL" << std::endl;
      reader_.Read(&request_, this);
      status = Status::READ;
    } else if (status == Status::READ && ok) {
      std::cout << "BatchHello: READ " << request_.name() << std::endl;
      ss << " " << request_.name();
      reader_.Read(&request_, this);
    } else if (status == Status::READ) {
      std::cout << "BatchHello: READ END" << std::endl;
      response_.set_greet(ss.str());
      reader_.Finish(response_, grpc::Status::OK, this);
      status = Status::DONE;
    } else if (status == Status::DONE) {
      std::cout << "BatchHello: DONE" << std::endl;
      new BatchHelloCallData(service_, cq_);
      delete this;
    } else {
      std::cerr << "BatchHello: UNKNOWN" << std::endl;
    }
  }

private:
  enum class Status {
    INIT,
    CALL,
    READ,
    DONE,
  };
  Status status = Status::INIT;
  bool done_ = false;
  grpc::ServerContext context_;
  grpc::ServerCompletionQueue* cq_;
  Hello::AsyncService* service_;
  HelloRequest request_;
  grpc::ServerAsyncReader<HelloResponse, HelloRequest> reader_;
  HelloResponse response_;
  std::stringstream ss;
};

void handle(grpc::ServerCompletionQueue* cq) {
  void* tag = nullptr;
  bool ok = false;
  while (true) {
    std::cout << "Handle" << std::endl;
    if (!cq->Next(&tag, &ok)) {
      // TODO: Need to retrieve remaining call data to delete.
      break;
    }
    static_cast<CallData*>(tag)->process(ok);
  }
}

int main(int, char**) {
  grpc::ServerBuilder builder;

  std::string addr("0.0.0.0:41384");
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());

  std::unique_ptr<grpc::ServerCompletionQueue> cq(builder.AddCompletionQueue());

  Hello::AsyncService service;
  builder.RegisterAsyncService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  new SayHelloCallData(&service, cq.get());
  new BatchHelloCallData(&service, cq.get());
  handle(cq.get());
  return 0;
}
