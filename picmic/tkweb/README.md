# PicoMic Web Interface (tkweb)

Interface web moderne et complète pour contrôler le système PicoMic DAQ via l'API REST FastAPI.

## Architecture

```
┌─────────────────────────────────────────────────┐
│          Web Browser (tkweb)                     │
│  ┌────────────────────────────────────────────┐  │
│  │  HTML + JavaScript + Bootstrap 5 + Chart.js│  │
│  │  - index.html (multi-tabs UI)              │  │
│  │  - app.js (API calls + Charts)             │  │
│  │  - styles.css (responsive design)          │  │
│  └────────────────────────────────────────────┘  │
└──────────────────┬────────────────────────────────┘
                   │ HTTP/REST API
                   │ (http://localhost:8000)
┌──────────────────▼────────────────────────────────┐
│          FastAPI Server (services)                 │
│  ┌────────────────────────────────────────────┐  │
│  │  main.py + picmic_routes.py                │  │
│  │  - Status Management                       │  │
│  │  - Configuration Management                │  │
│  │  - Calibration Control                     │  │
│  │  - Advanced Parameters                     │  │
│  │  - Method Introspection                    │  │
│  └────────────────────────────────────────────┘  │
└──────────────────┬────────────────────────────────┘
                   │ Direct calls
                   │
┌──────────────────▼────────────────────────────────┐
│      PicoMic Physical Layer (share)              │
│  ┌────────────────────────────────────────────┐  │
│  │  picmic_daq.py - Acquisition               │  │
│  │  picmic_storage.py - Data Storage          │  │
│  │  picmic_scurve.py - Calibration            │  │
│  │  Hardware Interface (DAQ, HV, MQTT, etc)   │  │
│  └────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────┘
```

## Installation & Démarrage

### 1. Démarrer le serveur FastAPI

```bash
cd /opt/pmdaq/picmic
python services/main.py
# Ou avec uvicorn:
# uvicorn services.main:app --host 0.0.0.0 --port 8000 --reload
```

### 2. Accéder à l'interface web

- **Version interactive**: http://localhost:8000/picmic (voir tkweb/index.html)
- **Documentation API Swagger**: http://localhost:8000/docs
- **Documentation API ReDoc**: http://localhost:8000/redoc

## Fichiers du Projet

| Fichier | Description |
|---------|-------------|
| `index.html` | Interface web principale (multi-tabs, responsive) |
| `app.js` | Logique JavaScript, API calls, charts |
| `styles.css` | Styles personnalisés (dark/light, responsive) |
| `README.md` | Cette documentation |
| `Dockerfile` | Déploiement Docker |
| `docker-compose.yml` | Stack Docker complet |

## Fonctionnalités

### 📊 Dashboard (Onglet 1)

- **Monitoring en temps réel** avec graphique d'événements
- **Status du système**:
  - État actuel (CREATED, INITIALISED, CONFIGURED, RUNNING)
  - Run counter
  - Event counter
  - État (IDLE/RUNNING)
- **Boutons de transition d'état**:
  - Initialize
  - Configure
  - Start
  - Stop
- **Logging** avec couleurs par niveau (info, success, warn, error)

### 🔍 Calibration (Onglet 2)

- **Paramètres de calibration S-curve**:
  - Threshold Min/Max
  - Threshold Step
  - DC_PA
  - Mode (optionnel)
- **Graphique S-curve** en temps réel avec Chart.js
- **Endpoints API**:
  - `POST /calibration` - Démarrer la calibration
  - `GET /calibration-data` - Récupérer les données

### ⚙️ Configuration (Onglet 3)

- **Sauvegarder configuration actuelle**:
  - Nom et description
  - Stockage en JSON dans `/tmp/picmic_configs/`
- **Liste des configurations sauvegardées**:
  - Charger configuration
  - Supprimer configuration
  - Voir date de modification
- **Endpoints API**:
  - `GET /config/list` - Lister les configs
  - `POST /config/save` - Sauvegarder
  - `GET /config/load/{name}` - Charger
  - `DELETE /config/{name}` - Supprimer

### 🛠️ Paramètres Avancés (Onglet 4)

- **DAQ Parameters**:
  - Threshold Value (0-4095)
  - DC_PA Value (0-255)
  - Enable Filtering (checkbox)
  - Enable Falling Edge (checkbox)
  - Force Valid Event (checkbox)
  - Negative Polarity (checkbox)
- **Application des paramètres** avec validation
- **Endpoints API**:
  - `GET /advanced-params` - Lire paramètres
  - `POST /advanced-params` - Appliquer paramètres

### 🧪 Methods (Onglet 5)

- **Méthodes dynamiques** chargées du serveur
- **Introspection automatique**:
  - Liste complète des méthodes disponibles
  - Signature et paramètres
  - Types de retour
- **Exécution avec modale**:
  - Génération dynamique de formulaires
  - Conversion de types automatique
  - Résultat et logs
- **Endpoints API**:
  - `GET /methods` - Lister méthodes
  - `POST /commands` - Exécuter méthode
  - `POST /transitions` - Exécuter transition FSM

## API Endpoints

### Status Management

```bash
GET /picmic/
GET /picmic/status-history
POST /picmic/update-status
```

### Configuration Management

```bash
GET /picmic/config/list
GET /picmic/config/load/{config_name}
POST /picmic/config/save?name=...&description=...
DELETE /picmic/config/{config_name}
```

### Calibration

```bash
POST /picmic/calibration
GET /picmic/calibration-data
```

### Advanced Parameters

```bash
GET /picmic/advanced-params
POST /picmic/advanced-params
```

### Methods & Transitions

```bash
GET /picmic/methods
POST /picmic/commands
POST /picmic/transitions
```

## Exemples d'utilisation

### Via cURL

```bash
# Charger les méthodes disponibles
curl http://localhost:8000/picmic/methods

# Exécuter une transition
curl -X POST http://localhost:8000/picmic/transitions \
  -H "Content-Type: application/json" \
  -d '{"name": "initialise", "params": {}}'

# Lister les configurations
curl http://localhost:8000/picmic/config/list

# Sauvegarder une configuration
curl -X POST "http://localhost:8000/picmic/config/save?name=TEST&description=Test Config"
```

### Via Python

```python
import requests

API_URL = "http://localhost:8000/picmic"

# Charger méthodes
response = requests.get(f"{API_URL}/methods")
methods = response.json()

# Exécuter une méthode
response = requests.post(f"{API_URL}/commands", json={
    "name": "some_method",
    "params": {"param1": "value"}
})
```

## Déploiement Docker

### Build et Run

```bash
cd /opt/pmdaq/picmic

# Build l'image
docker build -t picmic-web:latest -f tkweb/Dockerfile .

# Run avec docker-compose
docker-compose -f tkweb/docker-compose.yml up -d

# Accéder à http://localhost:8000
```

### Configuration Nginx

```nginx
server {
    listen 80;
    server_name picmic.example.com;

    location / {
        proxy_pass http://localhost:8000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
}
```

## Features & Améliorations

### ✅ Implémentées

- [x] Multi-tab interface responsive
- [x] Real-time monitoring avec Chart.js
- [x] Configuration management
- [x] Advanced parameters
- [x] Method introspection & execution
- [x] State machine transitions
- [x] Logging avec couleurs
- [x] S-curve visualization
- [x] CORS support
- [x] Error handling

### 📋 TODO (Future)

- [ ] WebSockets pour updates temps réel (vs polling)
- [ ] Authentication/Authorization
- [ ] Export data (CSV, PDF, JSON)
- [ ] Multi-user support
- [ ] Data persistence
- [ ] Performance optimization
- [ ] PWA support

## Troubleshooting

### Connexion refusée

```bash
# Vérifier que le serveur tourne
ps aux | grep main.py

# Vérifier le port
netstat -tlnp | grep 8000

# Relancer le serveur
cd /opt/pmdaq/picmic && python services/main.py
```

### CORS errors

Le CORS est activé pour tous les origins. Si problème, modifier `services/main.py`:

```python
app.add_middleware(
    CORSMiddleware,
    allow_origins=["https://yourdomain.com"],  # Spécifier domaines
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
```

### Charts ne s'affichent pas

- Vérifier que Chart.js est chargé: `https://cdn.jsdelivr.net/npm/chart.js`
- Vérifier la console du navigateur (F12) pour les erreurs
- S'assurer que les éléments `#chartMonitor` et `#chartScurve` existent dans le DOM

## Notes de développement

### Ajouter une nouvelle fonctionnalité

1. **Ajouter endpoint dans `picmic_routes.py`**:
```python
@router.get("/my-feature")
def my_feature():
    return {"result": "value"}
```

2. **Ajouter fonction dans `app.js`**:
```javascript
async function myFeature() {
    const response = await apiCall('/my-feature');
    // Traiter réponse
}
```

3. **Ajouter UI dans `index.html`**:
```html
<button onclick="myFeature()">My Feature</button>
```

## Support

Pour les bugs ou améliorations, contacter l'équipe DAQ.

---

**Version**: 1.1  
**Date**: Mai 2026  
**Status**: ✅ Production Ready
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
