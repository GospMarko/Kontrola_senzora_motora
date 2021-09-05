# KONTROLA SENZORA MOTORA
Potrebno je izsimulirati pristizanje vredosti 5 senzora motora automobila i nadzirati da one budu u nekim granicama.
-Simulirati vrednosti senzora pomocu serijske komunikacije AdvUniCom softerom. Takođe korišćene periferije LED bar plus i 7seg displej

# PERIFERIJE
-Prilikom pokretanja LED_bars_plus u komandnu liniju ukucati LED_bars_plus.exe rGB, jer je potrebno imati jedan stubac(prvi sa leva) kao ulazni ,a ostala dva stubca izlazni.

-Prilikom pokretanja serijske komunikacije, potrebno je aktivirati dva zasebna kanala AdvUniCom. U komandnu liniju kucati redom komande AdvUniCom.exe 0 , AdvUniCom.exe 1
Prilikom pokretanja sedmosegmentnog displeja kucati komandu Seg7_Mux.exe 10

# KAKO TESTIRATI SISTEM
Pokrenuti kanal 0 i uneti 5 vrednosti senzora u hex formatu (\00\62\55\97\01\02\ff npr.), to uneti u polje pored ok , čekirati kvadrat
auto pored i pritisnuti OK da se vrednosti automatski šalju. Ukoliko su vrednosti u granicama treći stubac LED bara neće biti uključen,a
ako nisu zatrepereće i javiti da je sistem u kritičnom stanju (ako je temperatura sa senzora rashladne tečnosti veća ili jednaka 110 stepeni, temperatura vazduha sa senzora usisne grane veća ili jednaka 90 stepeni, ako su obrtaji manji ili jedaki od 1500, odnosno veći ili jedaki 3000, ako je napon na senzoru opterećenja papučice veći ili jednak 4v i ako je napon sa senzora pedale gasa veći ili jedak 5V)
Pokrenuti kanal 1 i ueti hex komandu za mod. (\ef\4d\0d za MONITOR \ef\53\0d za SPEED i \ef\44\0d za DRIVE) kada se posalju ove naredbe na 7seg displeju će biti ispisani prvo mod pa trenutne vrednost 2 senzora koja odgovaraju modu).
Takođe manuelno je moguće birati modove pomoću 3 donja prekidača u prvom stupcu i identično ispisivati na 7seg displej. Donja dioda u drugom stupcu je ukljucna kada se upali neki taster, a iskljucena kad se iskljuci.


# OPIS TASKOVA 


# led_bar_tsk
-Ovaj task nam sluzi iskljucivo za proveru da li je pritisnut neki od donja 3 taster prvog stubca. Ukoliko jeste palimo skroz donju diodu drugog stubca kao indikaciju da je sistem upaljen, i ispisujemo mod i vrednost senzora za taj mod na 7seg displej.

# prvSerialReceiveTask_0
Prijem podataka sa kanala 0 serijske koomunikacije. 00 je pocetak poruke ff kraj. Podaci između se smeštaju u bafer niz i zatim se prilagođuju za dalji ispis na 7seg displeju. Takođe se porede primljeni podaci sa granicama sigurnog rada jer ako je neki parametar povišen ili snižen sistem je u kritičnom stanju.

# prvSerialReceiveTask_1
Prijem podataka sa kanala 1 serijske koomunikacije. ef je pocetak poruke 0d kraj. Podaci izmedju se smestaju u bafer1 niz i zatim uporediti koji je ascii karakter primljen da bi se odredio mod. Na osnovu moda će se ispisivati vrednosti senzora.

# SerialSend_Task

Task koji služi za slanje na oba kanala. Kanal nula će se slati automatski odgovarajućim trigerom , a kanal jedan manuelno.

