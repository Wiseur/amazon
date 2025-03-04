const int electromagnetPin = 8;  // Pin de l'électroaimant
const String targetBarcode = "PE0275";  // Code-barres pour désactivation pendant 3 secondes
const String degradedModeBarcode = "14810";  // Code-barres pour mode dégradé

unsigned long lastDegradedTime = 0;  // Variable pour suivre le temps du mode dégradé
const unsigned long degradedModeTimeout = 30000;  // Timeout du mode dégradé (30 secondes)
bool electromagnetActive = true;  // Variable pour suivre l'état de l'électroaimant
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
  
  // Initialisation de la pin de l'électroaimant
  pinMode(electromagnetPin, OUTPUT);
  
  Serial.println("Prêt à recevoir les données du lecteur de code-barres.");
}

void loop() {
  // Vérifier si des données sont disponibles sur le port série du lecteur de code-barres (Serial1)
  if (Serial1.available() > 0) {
    // Lire la donnée envoyée par le lecteur de code-barres
    String barcodeData = Serial1.readString();  // Lire les données envoyées par le lecteur
    
    // Nettoyer la chaîne (enlever les espaces et les retours à la ligne)
    barcodeData.trim();  // Enlève les espaces et les caractères de contrôle comme \n, \r
    
    // Afficher les données lues sur le moniteur série
    Serial.print("Code-barres scanné: ");
    Serial.println(barcodeData);  // Affiche la donnée (le code du produit scanné)
    
    // Vérifier si le code-barres scanné est le code cible pour désactiver l'électroaimant
    if (barcodeData == targetBarcode) {
      // Si le code-barres "PE0275" est scanné, désactiver l'électroaimant pendant 3 secondes
      deactivateElectromagnet();
    }
    
    // Vérifier si le code-barres scanné est pour activer ou désactiver le mode dégradé
    else if (barcodeData == degradedModeBarcode) {
      // Si le code-barres "14810" est scanné, activer/désactiver le mode dégradé
      toggleDegradedMode();
    }
  }
  
  // Si le mode dégradé est actif, désactiver l'électroaimant
  if (degradedModeActive) {
    electromagnetActive = false;
    analogWrite(electromagnetPin, 0);  // Éteindre l'électroaimant
  }
  
  // Si l'électroaimant est activé, envoyer un signal PWM
  else if (electromagnetActive) {
    analogWrite(electromagnetPin, 255);  // Valeur PWM pour la puissance maximale de l'électroaimant
  }
  
  // Vérifier si le mode dégradé est actif et que le délai de 30 secondes est écoulé
  if (degradedModeActive && (millis() - lastDegradedTime >= degradedModeTimeout)) {
    // Si 30 secondes sont passées, réactiver l'électroaimant
    degradedModeActive = false;  // Désactiver le mode dégradé
    electromagnetActive = true;  // Réactiver l'électroaimant
    Serial.println("Mode dégradé expiré. L'électroaimant est réactivé.");
  }
}

// Fonction pour désactiver l'électroaimant pendant 3 secondes
void deactivateElectromagnet() {
  electromagnetActive = false;  // Désactiver l'électroaimant
  analogWrite(electromagnetPin, 0);  // Éteindre l'électroaimant (PWM à 0)

  // Attendre 3 secondes (3000 ms)
  delay(3000);

  electromagnetActive = true;  // Réactiver l'électroaimant
}

// Fonction pour activer/désactiver le mode dégradé
void toggleDegradedMode() {
  if (degradedModeActive) {
    // Si le mode dégradé est déjà actif, désactiver
    degradedModeActive = false;
    electromagnetActive = true;  // Réactiver l'électroaimant
    Serial.println("Mode dégradé désactivé. L'électroaimant est réactivé.");
  } else {
    // Si le mode dégradé n'est pas actif, l'activer
    degradedModeActive = true;
    electromagnetActive = false;  // Désactiver l'électroaimant
    lastDegradedTime = millis();  // Enregistrer le moment où le mode dégradé a été activé
    Serial.println("Mode dégradé activé. L'électroaimant est désactivé.");
  }
}
