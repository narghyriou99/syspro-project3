Earino Eksamino 2021
Programmatismos Systhmatos
Ergasia 3
Argyriou Nikolaos
AM: 1115201700008

H epikoinwnia over the network exei ulopoihthei ws eksis:

O travelMonitorClient paizei ton rolo tou Client kai antistoixa oi monitorServers paizoun ton rolo twn Servers.
Sigkekrimena dimiourgw numMonitors sockets kai servers wste na sindethoun me ton client


-Queries:
1)Leitourgei swsta
2)Leitourgei swsta
3)Leitourgei swsta
4)Leitourgei swsta
5)Leitourgei swsta

Endexetai meta tin ektelesi merikwn queries, na xreiastei o xristis na patisei to "Enter" sto pliktrologio gia na emfanistoun oi epiloges.(DEN APAITEITAI PANTA)

-Log files: Ola ta logs dimiourgountai sto directory logs

-Threads: Arxika kaleitai numThread fores h pthread_create me thn consume, h opoia analamvanei na elegxei an o cyclic buffer exei filepaths etsi wste na ta diavasei.

Epeita, ginetai insert twn directories kai twn .txts se mia geniki lista (directoryList) kai tautoxrona ginetai attempt gia place ston cyclic buffer.

Afou to monitorServer perimenei na ginoun parse ola ta txs, epeita stelnei ta Bloom filters kai ena minima "READY" kai katopin perimenei stin read.

Ta threads "koimountai", diladi kollane stin pthread_cond_wait kai "ksipnane" mono an mpoun kainouria filepaths mesa ston cyclicBuffer mesw toy 3ou query. 

Ta threads "pethainoun" me thn kill, otan o xrhsths dwsei tin entoli "/exit".

Odigies Ektelesis:

Simperilamvanontai 2 fakeloi me arxeia. To ./small_dir exei liges eggrafes(gia elegxo apantisewn) kai to ./big_dir exei megalo plithos eggrafwn(gia elegxo scalability)

-Makefile:  make: separate compilation
            make run: ./travelMonitorClient -m 10 -b 16 -c 10 -s 100000 -i ./big_dir -t 5
            make valgrind: valgrind ./travelMonitorClient -m 10 -b 16 -c 10 -s 100000 -i ./big_dir -t 5
            make clean: Ekkatharisi .o, ektelesimwn kai log files
            make count: Sinolikes grammes kwdika

-Paratiriseis:
Exw paratirisei pws merikes fores to programma exei undefined simperifora, diladi gia paradeigma mporei na kanoun "fail" kapoies read functions twn bloom filters
kai etsi meta epireazontai oles oi alles read functions me apotelesma na kolisei kai na xreiastei CTRL + C gia na stamatisei. Gia auto ton logo mono stin "lipsi" twn bloom filters
apo ton travelMonitorClient exw xrisimopoihsei tin sinartisi recv() me flag MSG_WAITALL, opou ipotithetai midenizei sxedon tis pithanotites na kanei "fail" kapoia read.