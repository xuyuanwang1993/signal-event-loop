package main

import (
	"crypto/rand"
	"fmt"
	"math/big"
	"net"
)
import "time"
import "os"
import "strconv"

func main() {
	fmt.Println("hello world")
	testDisk()
	testOpt()
	testMem()
	testNetwork()
}

func testNetwork() {
	testCnt := 10000000
	fmt.Printf("test mem malloc start NETWORK_CNT=%d\r\n", testCnt)
	st := time.Now()
	if ptr, ok := os.LookupEnv("NETWORK_CNT"); ok {
		testCnt, _ = strconv.Atoi(ptr)
	}
	buf := make([]byte, 1024)
	fd1, _ := net.ListenUDP("udp", &net.UDPAddr{
		IP:   net.IPv4(127, 0, 0, 1),
		Port: 0,
	})
	fd2, _ := net.ListenUDP("udp", &net.UDPAddr{
		IP:   net.IPv4(127, 0, 0, 1),
		Port: 50000,
	})
	execCnt := testCnt
	for execCnt > 0 {
		execCnt--
		_, err3 := fd1.WriteToUDP(buf, &net.UDPAddr{
			IP:   net.IPv4(127, 0, 0, 1),
			Port: 50000,
		})
		if err3 != nil {
			fmt.Println("send failed", err3)
		}
		_, err4 := fd2.Read(buf)
		if err4 != nil {
			fmt.Println("recv failed", err4)
		}
	}
	eT := time.Since(st)
	fmt.Printf("test mem malloc finish NETWORK_CNT=%d  cost [%d]ms\r\n", testCnt, eT.Milliseconds())
}

func testDisk() {
	testCnt := 10000
	fmt.Printf("test mem malloc start DISK_CNT=%d\r\n", testCnt)
	st := time.Now()
	if ptr, ok := os.LookupEnv("DISK_CNT"); ok {
		testCnt, _ = strconv.Atoi(ptr)
	}
	var buf []byte = make([]byte, 1024)
	file, err := os.OpenFile("test.wb", os.O_CREATE|os.O_TRUNC|os.O_WRONLY, 0666)
	if err != nil {
		fmt.Println("open failed", err)
		return
	}
	defer os.Remove("test.wb")
	execCnt := testCnt
	for execCnt > 0 {
		execCnt--
		file.Write(buf)
	}
	file.Close()
	eT := time.Since(st)
	fmt.Printf("test mem malloc finish DISK_CNT=%d  cost [%d]ms\r\n", testCnt, eT.Milliseconds())
}

func testOpt() {
	testCnt := 10000000
	fmt.Printf("test mem malloc start OPERATION_CNT=%d\r\n", testCnt)
	st := time.Now()
	if ptr, ok := os.LookupEnv("OPERATION_CNT"); ok {
		testCnt, _ = strconv.Atoi(ptr)
	}
	execCnt := testCnt
	var result float64 = 0.0
	for execCnt > 0 {
		execCnt--
		option1, _ := rand.Int(rand.Reader, big.NewInt(10000000))
		option2, _ := rand.Int(rand.Reader, big.NewInt(10000000))
		result = result + (float64(option1.Uint64())/13.0)*(float64(option2.Uint64())/17.0)
	}
	eT := time.Since(st)
	fmt.Printf("test mem malloc finish OPERATION_CNT=%d result[%f] cost [%d]ms\r\n", testCnt, result, eT.Milliseconds())
}

func testMem() {
	testCnt := 10000000
	fmt.Printf("test mem malloc start MEM_CNT=%d\r\n", testCnt)
	st := time.Now()
	if ptr, ok := os.LookupEnv("MEM_CNT"); ok {
		testCnt, _ = strconv.Atoi(ptr)
	}
	gap := testCnt / 10
	execCnt := testCnt
	for execCnt > 0 {
		execCnt--
		buf := make([]byte, 1024)
		if execCnt%gap == 0 {
			fmt.Printf("gap=%d ptr=%p\r\n", gap, buf)
		}
	}
	eT := time.Since(st)
	fmt.Printf("test mem malloc finish MEM_CNT=%d  cost [%d]ms\r\n", testCnt, eT.Milliseconds())
}
