# GL-Man

**GL-Man** è un progetto sviluppato nell'ambito del corso di **Grafica Tridimensionale Avanzata (2023-2024)** presso l'Università degli Studi della Basilicata, laurea magistrale in **Ingegneria Informatica e delle Tecnologie dell'Informazione**. 

Si tratta di un'implementazione **3D non ufficiale** del classico videogioco **Pac-Man** (1980), ideato da Toru Iwatani e prodotto dalla Namco, sviluppata in **C++** e basata sull'API grafica **OpenGL**.

---

### **Motore di rendering 3D integrato**

Durante lo sviluppo di **GL-Man** è stato progettato e implementato un **motore di rendering 3D modulare** che fornisce le fondamenta grafiche per il gioco. Questo motore, pur non dotato di un'interfaccia grafica, consiste in una collezione di classi generiche e riutilizzabili, progettate per gestire gli aspetti principali della grafica tridimensionale. 

#### **Caratteristiche Principali**

- **Struttura Modulare**<br />
  Le classi sono organizzate per fornire funzionalità specifiche in un contesto flessibile con una netta separazione delle responsabilità per garantire la riusabilità del codice e la scalabilità per progetti futuri 
  <br />

- **Gestione del File System**<br />
  Per semplificare l'accesso e la gestione delle risorse di progetto, è stata sviluppata una classe dedicata per l'interazione con il file system del sistema operativo Windows, ovvero la classe `FileSystem`. 
  Questa classe è progettata per astrarre le operazioni comuni di lettura e scrittura di file, garantendo flessibilità e portabilità.
  <br />

- **Gestione delle Risorse**<br />
  Tutte le risorse utilizzate sono caricate, memorizzate e gestite tramite la classe `ResourceManager`, la quale ha le seguenti funzionalità:

  - Sistema di caricamento, compilazione e memorizzazione degli shader ***GLSL***, inclusi vertex, fragment e (opzionalmente) geometry shader. Con supporto per tutti i principali formati come `.vs`, `.fs` e `.gs`.
  
  - Sistema di caricamento e memorizzazione delle texture, basato sulla libreria ***stb_image*** con supporto per diverse modalità di filtraggio e wrapping.
  
  - Sistema di caricamento e memorizzazione di modelli 3D e dei loro materiali, basato sulla libreria ***Assimp***, con supporto per il formato standard `.obj`.
  <br />

- **Gestione del Logging**<br />
  Tutte le stampe sono effettuate tramite la classe `LoggerManager`, basata sulla libreria ***spdlog***. La classe `LoggerManager` consente di: registrare messaggi su diversi livelli di log; utilizzare messaggi formattati dinamicamente; stampare utilizzando un thread dedicato e separato da quello di rendering, in modo da non rallentare quest'ultimo.
  <br />

- **Gestione dei Game Object**<br />
  Per il motore di rendering ogni elemento disegnato a schermo è una istanza della classe `GameObjectBase`. 
  Tale classe tramite le sue sottoclassi `GameObjectCustom` e `GameObjectFromModel` permette il corretto posizionamento e disegno di ogni Game Object. 
  `GameObjectCustom` è utile per definire un Game Object date mesh, diffuseTexture e specularTexture. Mentre `GameObjectFromModel` è utile per definire un Game Object dato un modello. 
  Entrambe le classi precedenti permettono di generare la OBB del Game Object. Una Oriented Bounding Box (OBB) è una rappresentazione compatta del volume di delimitazione, che si adatta strettamente alle geometrie che rappresenta. L'invarianza di un OBB alla traslazione e alla rotazione lo rende ideale come rappresentazione ottimale e predefinita del volume di delimitazione in uno spazio tridimensionale. È inoltre fondamentale per implementare un Collider preciso e ottimizzato.
  <br />

- **Gestione degli effetti di Post-Processing**<br />
  Tutti gli effetti di Post-Processing vengono gestiti dalla classe `PostProcessor`, che  gestisce le operazioni di post-processing nel rendering, come HDR, Bloom e anti-aliasing multisample (MSAA). 
  Il `PostProcessor` è pensato per ottimizzare la qualità visiva, integrandosi con gli shader e migliorando il realismo delle scene.
  <br />

- **Gestione del Testo in Sovrimpressione**<br />
  Il testo in sovrimpressione viene gestito dalla classe `TextRenderer`, la quale è responsabile del rendering del testo 2D utilizzando font caricati tramite la libreria **FreeType**. 
  Il `TextRenderer` consente di aggiungere testo stilizzato alle scene 3D o 2D, come punteggi, HUD, o notifiche, garantendo flessibilità e alta qualità visiva.
  <br />

#### **Utilizzo generale**
Questo motore è stato progettato per essere indipendente dal progetto specifico del gioco. Pur essendo utilizzato come base per lo sviluppo di **GL-Man**, può essere facilmente integrato in altri progetti che richiedono funzionalità di rendering 3D. 

Grazie a questa struttura flessibile, il motore rappresenta un valido punto di partenza per progetti accademici, prototipi o applicazioni più complesse nel campo della grafica tridimensionale.

#### **Nota Bene**
Il motore, essendo privo di un'interfaccia grafica dedicata, richiede conoscenze di programmazione in **C++** e familiarità con le API di **OpenGL** per essere utilizzato e personalizzato. Ulteriori dettagli sulle sue funzionalità possono essere trovati nei commenti del codice.

---

### **Gioco**

Il gioco integra diversi concetti e tecniche avanzate di grafica tridimensionale:

- **Depth Testing**: Abilitato per migliorare la resa visiva e ottimizzare le prestazioni.
- **Stencil Testing**: Utilizzato per evidenziare visivamente il modello del power-up.
- **Face Culling**: Abilitato per migliorare le prestazioni di rendering, escludendo i poligoni nascosti dal frustum di visione.
- **Alpha Blending**: Applicato ai fantasmi quando si trovano in stato vulnerabile.
- **Uniform Buffer Objects**: Adoperati per ottimizzare le prestazioni del rendering.
- **Interface Block**: Adoperati per ottimizzare lo scambio di informazioni tra vertex e fragment shader.
- **Instancing**: Usato per rappresentare efficientemente muri, pavimento, dot e energizer.
- **Anti-Aliasing(MSAA)**: L'utente può abilitare o disabilitare l'MSAA per una grafica più fluida.
- **Illuminazione avanzata**: Shader con Blinn-Phong e correzione gamma per una resa realistica.
- **High Dynamic Range(HDR)**: L'utente può abilitare o disabilitare la visualizzazione in HDR per una grafica più profonda.
- **Bloom**: Utilizzato per enfatizzare visivamente gli energizer.

Il gioco include inoltre una interfaccia grafica utente sviluppata con **Dear ImGui**, che permette all'utente di impostare parametri come HDR e anti-aliasing. 
Questo sistema consente una facile configurazione delle opzioni grafiche per adattare il gioco alle preferenze dell'utente e alle capacità del sistema.

---

### Dipendenze principali del progetto:

- Assimp
- FreeType
- Glad
- GLFW
- GLM
- Dear ImGUI
- IrrKlang
- Spdlog
- STB

---

### **Utilizzo**

Nella cartella `bin` presente nella radice di progetto è prwesente un file eseguibile, risultato della compilazione del progetto in ambiente Windows 10, CPU Intel x86.

In caso di problemi è possibile ricompilare il progetto in maniera semplificata aprendo il file `.sln` con Visual Studio.

---

### **Gallery**

|  |  |
|--|--|
| Home Screen <BR>![home](./res/screenshots/screenshot_home.png) | Settings Screen - Anti-Aliasing <BR>![home](./res/screenshots/screenshot_settings_anti-aliasing.png) |
|Settings Screen - HDR <BR>![home](./res/screenshots/screenshot_settings_hdr.png) | Game Screen <BR>![home](./res/screenshots/screenshot_game.png) |

---

### **Contributi**

Contributi e suggerimenti sono benvenuti! Apri un issue per segnalare problemi o invia una pull request per proporre miglioramenti😁.

---

### **License**

GL-Man is licensed under the MIT License, see [LICENSE.txt](./LICENSE.txt) for more information.
