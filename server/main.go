package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"math/rand"
	"net"

	pb "grpcexp/numbers"

	"google.golang.org/grpc"
)

var (
	port = flag.Int("port", 3000, "The server port")
)

type service struct {
	pb.UnimplementedNumbersServer
}

func (s *service) GetRandom(ctx context.Context, r *pb.GetRandomRequest) (*pb.GetRandomReply, error) {
	num := rand.Int31n(r.Highest)
	return &pb.GetRandomReply{Result: num}, nil
}

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterNumbersServer(s, &service{})
	log.Printf("server listening at %v", lis.Addr())
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
