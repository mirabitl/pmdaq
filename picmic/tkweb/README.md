# PicoMic Web Interface (tkweb)

Interface web moderne pour contrôler le système PicoMic DAQ via l'API REST FastAPI.

## Architecture

Cette interface remplace l'interface TK desktop (répertoire `tk`) en utilisant une architecture client-serveur:

```
┌─────────────────────────────────────────────────┐
│          Web Browser (tkweb)                     │
│  ┌────────────────────────────────────────────┐  │
│  │  HTML + JavaScript + Bootstrap 5           │  │
│  │  - index.html                              │  │
│  │  - app.js (API calls)                      │  │
│  │  - styles.css                              │  │
│  └────────────────────────────────────────────┘  │
└──────────────────┬────────────────────────────────┘
                   │ HTTP/REST API
                   │ (http://localhost:8000)
┌──────────────────▼────────────────────────────────┐
│          FastAPI Server (services)                 │
│  ┌────────────────────────────────────────────┐  │
│  │  main.py + picmic_routes.py                │  │
│  │  - /picmic/                                │  │
│  │  - /picmic/methods                         │  │
│  │  - /picmic/commands                        │  │
│  │  - /picmic/create                          │  │
│  └────────────────────────────────────────────┘  │
└──────────────────┬────────────────────────────────┘
                   │ Direct calls
                   │
┌──────────────────▼────────────────────────────────┐
│      PicoMic Physical Layer (services)            │
│  ┌────────────────────────────────────────────┐  │
│  │  picmic_models.py (picmic_physic class)    │  │
│  │  - Calibration methods                     │  │
│  │  - S-curve methods                         │  │
│  │  - DAQ control methods                     │  │
│  └────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────┘
```

## Installation

### 1. Assurez-vous que le serveur FastAPI est en cours d'exécution

```bash
cd /opt/pmdaq/picmic/services
python main.py
# ou avec uvicorn
uvicorn main:app --host 0.0.0.0 --port 8000
```

### 2. Ouvrir l'interface web

- **Localement**: http://localhost:8000/docs (Swagger UI)
- **Interface personnalisée**: Ouvrez `index.html` dans un navigateur

### 3. (Optionnel) Servir via un serveur web

```bash
# Avec Python
cd /opt/pmdaq/picmic/tkweb
python -m http.server 8080

# Avec Apache/Nginx
# Configurez un virtualhost pour servir le répertoire tkweb
```

## Fichiers

- **index.html** - Interface web principale (HTML5 responsive)
- **app.js** - Logique JavaScript pour appeler l'API REST
- **styles.css** - Styles CSS personnalisés
- **README.md** - Cette documentation

## Fonctionnalités

### 1. Configuration

- **Create Configuration**: Charger une configuration de la base de données
  - Paramètres: `name`, `version`
  - Exemple: "LIROC_TEST", version 1

### 2. Status en temps réel

- **Auto-refresh**: Mise à jour automatique du status toutes les 2 secondes
- Affiche:
  - État actuel (CREATED, INITIALISED, CONFIGURED, RUNNING)
  - Numéro de run
  - Nombre d'événements
  - État de fonctionnement (IDLE/RUNNING)

### 3. Methods

L'interface charge dynamiquement la liste des méthodes disponibles du serveur:

- **Calibration**:
  - `align()` - Calibration du seuil
  - `calib_dac_local()` - Calibration DAC local
  - `calib_iterative_dac_local()` - Calibration itérative

- **S-Curves**:
  - `sweep_dac10b()` - Balayage DAC
  - `scurve_single_chan()` - S-curve pour un canal
  - `scurve_all_channels()` - S-curves tous canaux
  - `scurve_loop_one()` - S-curves un par un
  - `start_scurves()` - Démarrer les mesures
  - `scurve_run()` - Exécution complète

- **Transitions d'état**:
  - `initialise()` - Initialiser
  - `configure()` - Configurer
  - `start()` - Démarrer
  - `stop()` - Arrêter

### 4. Exécution des méthodes

- Cliquez sur le bouton d'une méthode
- Remplissez les paramètres dans le modal
- Cliquez "Execute"
- Consultez le résultat dans le log

## Configuration de l'API

### URL de base

Par défaut: `http://localhost:8000`

Modifiez dans `app.js`:
```javascript
const API_BASE_URL = 'http://localhost:8000';
```

### Paramètres de l'API

Pour des scénarios personnalisés, modifiez les constantes dans `app.js`:
```javascript
const STATUS_UPDATE_INTERVAL = 2000; // ms entre les refreshes
```

## Avantages par rapport à l'interface TK

✅ **Responsive**: Fonctionne sur desktop, tablette, mobile
✅ **Moderne**: Interface utilisateur intuitive avec Bootstrap 5
✅ **Distribué**: Client et serveur peuvent être sur des machines différentes
✅ **Sans dépendances graphiques**: Pas besoin de X11 ou de libraries TK
✅ **Multi-utilisateurs**: Plusieurs clients peuvent se connecter au même serveur
✅ **Facilement extensible**: Ajoutez des fonctionnalités en JavaScript/HTML

## Exemples d'utilisation

### Calibration complète

1. Create Configuration → "LIROC_TEST", v1
2. Aller dans Transitions → `initialise()`
3. Aller dans Transitions → `configure()`
4. Aller dans Methods → `align()` → thmin=700, thmax=900
5. Attendre la fin
6. Consulter le log pour les résultats

### Mesure S-curve

1. Create Configuration → sélectionner votre config
2. Configure → `configure()`
3. Methods → `start_scurves()` → 
   - analysis: "SCURVE_A"
   - thmin: 700
   - thmax: 900
   - thstep: 1
4. Attendre la fin et consulter les résultats

## Troubleshooting

### "Disconnected" (rouge)

- Vérifiez que le serveur FastAPI est en cours d'exécution
- Vérifiez l'URL dans `app.js`
- Vérifiez les logs du serveur

### Les méthodes ne s'affichent pas

- Rafraîchissez la page (F5)
- Vérifiez la console JavaScript (F12)
- Vérifiez que `/picmic/methods` est accessible

### Les commandes ne s'exécutent pas

- Vérifiez que le `state` actuel le permet
- Consultez le log pour les messages d'erreur
- Vérifiez que tous les paramètres sont remplis

## Développement

### Ajouter une nouvelle fonction

1. Ajoutez la méthode à `picmic_models.py`
2. Elle apparaîtra automatiquement dans le web UI
3. Les paramètres sont parsés automatiquement

### Personnaliser l'interface

- Modifiez `index.html` pour ajouter des éléments
- Modifiez `app.js` pour ajouter de la logique
- Modifiez `styles.css` pour les styles

## API REST directe

Vous pouvez aussi utiliser l'API REST directement:

```bash
# Obtenir le status
curl http://localhost:8000/picmic/

# Lister les méthodes
curl http://localhost:8000/picmic/methods

# Exécuter une commande
curl -X POST http://localhost:8000/picmic/commands \
  -H "Content-Type: application/json" \
  -d '{"name": "initialise", "params": {}}'

# Créer une configuration
curl -X POST http://localhost:8000/picmic/create \
  -H "Content-Type: application/json" \
  -d '{"name": "LIROC_TEST", "version": 1}'
```

## Support

Pour des problèmes ou améliorations, consultez:
- Les logs du serveur FastAPI
- La console JavaScript du navigateur (F12)
- Les documentations de FastAPI et Bootstrap 5
