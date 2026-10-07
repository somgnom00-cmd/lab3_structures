dbms: *.go go.mod
	go build -buildvcs=false -o dbms .

clean:
	rm -f dbms
