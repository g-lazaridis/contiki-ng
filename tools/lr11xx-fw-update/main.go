package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"strconv"

	"github.com/azurity/xmodem-go"
	"go.bug.st/serial"
)

const usage = "usage: xmodem-go [ -f filename -d ttyPort -b BaudRate]"

var fileDir *string = flag.String("f", "", "File to send")
var ttyPort *string = flag.String("d", "", "TTY port to send file")
var baudrate *string = flag.String("b", "", "baudrate")

func main() {

	flag.Parse()

	if *fileDir == "" || *ttyPort == "" || *baudrate == "" {
		fmt.Println(usage)
		return
	}

	br, err := strconv.Atoi(*baudrate)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	connection, err := serial.Open(*ttyPort, &serial.Mode{BaudRate: br})
	if err != nil {
		log.Fatalln(err)
	}

	// Open file
	fIn, err := os.Open(*fileDir)
	if err != nil {
		log.Fatalln(err)
	}

	fn := xmodem.ModemXMin | xmodem.ModemFnCRC | xmodem.ModemFnCANCAN
	conf := xmodem.XModemConfig(fn)

	m, _, _ := xmodem.NewModem(conf, connection, connection)

	err = m.SendBytes(fIn)
	if err != nil {
		log.Panicln(err)
	}

	fIn.Close()

	log.Println("sent successful")

}
