const int electromagnetPin1 = 8;  // Pin du premier électroaimant
const int electromagnetPin2 = 9;  // Pin du deuxième électroaimant
const String targetBarcode1 = "PE0275";  // Code-barres pour désactiver le premier électroaimant
const String targetBarcode2 = "PE0513";  // Code-barres pour désactiver le deuxième électroaimant
const String degradedModeBarcode = "14810";  // Code-barres pour mode dégradé

unsigned long lastDegradedTime = 0;  // Variable pour suivre le temps du mode dégradé
const unsigned long degradedModeTimeout = 30000;  // Timeout du mode dégradé (30 secondes)
bool electromagnetActive1 = true;  // Variable pour suivre l'état du premier électroaimant
bool electromagnetActive2 = true;  // Variable pour suivre l'état du deuxième électroaimant
bool degradedModeActive = false;  // Indicateur pour savoir si le mode dégradé est activé

void setup() {
  // Initialisation de la communication série avec l'ordinateur
  Serial.begin(9600);  // Communication avec le moniteur série
  
  // Initialisation de la communication série avec le lecteur de code-barres
  Serial1.begin(9600);  // Le taux de transmission dépend du lecteur de code-barres
  
  // Attendre que tout soit prêt
  while (!Serial) {
    ;  // Attendre que la connexion série soit prête
  }
  
  // Initialisation des pins des électroaimants
  pinMode(electromagnetPin1, OUTPUT);
  pinMode(electromagnetPin2, OUTPUT);
  
  Serial.println("Prêt à recevoir les données du lecteur de code-barres.");
}

void loop() {
  // Vérifier si des données sont disponibles sur le port série du lecteur de code-barres (Serial1)
  if (Serial1.available() > 0) {
    // Lire la donnée envoyée par le lecteur de code-barres
    String barcodeData = Serial1.readString();  // Lire les données envoyées par le lecteur
    
    // Nettoyer la chaîne (enlever les espaces et les retours à la ligne)
    barcodeData.trim();  // Enlève les espaces et les caractères de contrôle comme \n, \r
    
    // Afficher les données lues sur le moniteur série pour débogage
    Serial.print("Code-barres scanné: ");
    Serial.println(barcodeData);  // Affiche la donnée (le code du produit scanné)
    
    // Vérifier si le code-barres scanné est pour désactiver le premier électroaimant
    if (barcodeData == targetBarcode1) {
      // Si le code-barres "PE0275" est scanné, désactiver le premier électroaimant pendant 3 secondes
      deactivateElectromagnet1();
    }
    
    // Vérifier si le code-barres scanné est pour désactiver le deuxième électroaimant
    else if (barcodeData == targetBarcode2) {
      // Si le code-barres "PE0513" est scanné, désactiver le deuxième électroaimant pendant 3 secondes
      deactivateElectromagnet2();
    }
    
    // Vérifier si le code-barres scanné est pour activer ou désactiver le mode dégradé
    else if (barcodeData == degradedModeBarcode) {
      // Si le code-barres "14810" est scanné, activer/désactiver le mode dégradé
      toggleDegradedMode();
    }
  }
  
  // Si le mode dégradé est actif, désactiver les électroaimants
  if (degradedModeActive) {
    electromagnetActive1 = false;
    electromagnetActive2 = false;
    analogWrite(electromagnetPin1, 0);  // Éteindre le premier électroaimant
    analogWrite(electromagnetPin2, 0);  // Éteindre le deuxième électroaimant
  }
  
  // Si le premier électroaimant est activé, envoyer un signal PWM
  else if (electromagnetActive1) {
    analogWrite(electromagnetPin1, 255);  // Valeur PWM pour la puissance maximale du premier électroaimant
  }
  
  // Si le deuxième électroaimant est activé, envoyer un signal PWM
  if (electromagnetActive2) {
    analogWrite(electromagnetPin2, 255);  // Valeur PWM pour la puissance maximale du deuxième électroaimant
  }
  
  // Vérifier si le mode dégradé est actif et que le délai de 30 secondes est écoulé
  if (degradedModeActive && (millis() - lastDegradedTime >= degradedModeTimeout)) {
    // Si 30 secondes sont passées, réactiver les électroaimants
    degradedModeActive = false;  // Désactiver le mode dégradé
    electromagnetActive1 = true;  // Réactiver le premier électroaimant
    electromagnetActive2 = true;  // Réactiver le deuxième électroaimant
    Serial.println("Mode dégradé expiré. Les électroaimants sont réactivés.");
  }
}

// Fonction pour désactiver le premier électroaimant pendant 3 secondes
void deactivateElectromagnet1() {
  electromagnetActive1 = false;  // Désactiver le premier électroaimant
  analogWrite(electromagnetPin1, 0);  // Éteindre le premier électroaimant (PWM à 0)

  // Attendre 3 secondes (3000 ms)
  delay(3000);

  electromagnetActive1 = true;  // Réactiver le premier électroaimant
}

// Fonction pour désactiver le deuxième électroaimant pendant 3 secondes
void deactivateElectromagnet2() {
  electromagnetActive2 = false;  // Désactiver le deuxième électroaimant
  analogWrite(electromagnetPin2, 0);  // Éteindre le deuxième électroaimant (PWM à 0)

  // Attendre 3 secondes (3000 ms)
  delay(3000);

  electromagnetActive2 = true;  // Réactiver le deuxième électroaimant
}

// Fonction pour activer/désactiver le mode dégradé
void toggleDegradedMode() {
  if (degradedModeActive) {
    // Si le mode dégradé est déjà actif, désactiver
    degradedModeActive = false;
    electromagnetActive1 = true;  // Réactiver le premier électroaimant
    electromagnetActive2 = true;  // Réactiver le deuxième électroaimant
    Serial.println("Mode dégradé désactivé. Les électroaimants sont réactivés.");
  } else {
    // Si le mode dégradé n'est pas actif, l'activer
    degradedModeActive = true;
    electromagnetActive1 = false;  // Désactiver le premier électroaimant
    electromagnetActive2 = false;  // Désactiver le deuxième électroaimant
    lastDegradedTime = millis();  // Enregistrer le moment où le mode dégradé a été activé
    Serial.println("Mode dégradé activé. Les électroaimants sont désactivés.");
  }
}
