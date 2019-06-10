# Transmetteur-SMS-alarme
Transmission des messages et alarmes de l'alarme ARITECH CD 34 via SMS

Un ARDUINO avec une carte fille SIM800 est raccordé sur le bus clavier de l'alarme ARITECH CD34
il écoute les messages envoyés entre la boitier et les claviers et envoie un SMS lorsqu'il détecte l'allumage de la LED orange (défaut alarme) ou de la LED rouge (Alarme) puis au retour à la normale.

Le code Arduino utilise la bibliothèque FONA de Adafruit et est basé sur le code développé par Ozmo
