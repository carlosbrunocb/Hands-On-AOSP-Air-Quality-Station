package com.airstation.app.ui.settings

import android.Manifest
import android.os.Build
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    onBack: () -> Unit,
    viewModel: SettingsViewModel = hiltViewModel()
) {
    val notificationsEnabled by viewModel.notificationsEnabled.collectAsState()
    val pm25Threshold by viewModel.pm25Threshold.collectAsState()
    val coThreshold by viewModel.coThreshold.collectAsState()
    val exportMessage by viewModel.exportMessage.collectAsState()
    val context = LocalContext.current

    // Observa mudanças na mensagem de exportação para mostrar o Toast
    LaunchedEffect(exportMessage) {
        exportMessage?.let { msg ->
            Toast.makeText(context, msg, Toast.LENGTH_LONG).show()
            viewModel.clearExportMessage() // Limpa o estado após exibir
        }
    }

    val permissionLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.RequestPermission(),
        onResult = { isGranted -> if (isGranted) viewModel.toggleNotifications(true) }
    )

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Configurações") },
                navigationIcon = {
                    IconButton(onClick = onBack) { Icon(Icons.AutoMirrored.Filled.ArrowBack, "Voltar") }
                }
            )
        }
    ) { paddingValues ->
        Column(modifier = Modifier.padding(paddingValues).padding(16.dp)) {

            // --- BLOCO 1: NOTIFICAÇÕES ---
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween, verticalAlignment = Alignment.CenterVertically) {
                Text("Notificações de Alerta", style = MaterialTheme.typography.titleMedium)
                Switch(
                    checked = notificationsEnabled,
                    onCheckedChange = { isChecked ->
                        if (isChecked && Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                            permissionLauncher.launch(Manifest.permission.POST_NOTIFICATIONS)
                        } else {
                            viewModel.toggleNotifications(isChecked)
                        }
                    }
                )
            }
            Text("Avisa se os níveis de poluição ou gás letal ficarem perigosos.", style = MaterialTheme.typography.bodySmall, color = Color.Gray)

            Spacer(modifier = Modifier.height(24.dp))

            // --- BLOCO 2: PM2.5 ---
            Card(modifier = Modifier.fillMaxWidth(), colors = CardDefaults.cardColors(containerColor = Color.White), elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                        Text("Alerta Poluição (PM2.5):")
                        Text("${pm25Threshold.toInt()} µg/m³", fontWeight = FontWeight.Bold, color = if (pm25Threshold > 50) Color.Red else Color.Unspecified)
                    }
                    Slider(value = pm25Threshold, onValueChange = { viewModel.updateThreshold(it) }, valueRange = 10f..100f, steps = 9)
                    Text(text = "Recomendado OMS: 35 µg/m³", style = MaterialTheme.typography.labelSmall, modifier = Modifier.align(Alignment.CenterHorizontally))
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // --- BLOCO 3: CO LETHAL ---
            Card(modifier = Modifier.fillMaxWidth(), colors = CardDefaults.cardColors(containerColor = Color.White), elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                        Text("Alarme Monóxido Carbono:")
                        Text("${coThreshold.toInt()} ppm", fontWeight = FontWeight.Bold, color = Color.Red)
                    }
                    Slider(value = coThreshold, onValueChange = { viewModel.updateCoThreshold(it) }, valueRange = 10f..100f, steps = 9)
                    Text(text = "Gás Inodoro e Letal. Recomendado: < 50 ppm", style = MaterialTheme.typography.labelSmall, color = Color.Red, modifier = Modifier.align(Alignment.CenterHorizontally))
                }
            }

            Spacer(modifier = Modifier.height(32.dp))

            // --- BLOCO 4: EXPORTAR CSV (EXCEL) ---
            Text("Dados e Histórico", style = MaterialTheme.typography.titleMedium)
            Text("Exporte os dados dos sensores para análise em planilhas.", style = MaterialTheme.typography.bodySmall, color = Color.Gray)

            Spacer(modifier = Modifier.height(8.dp))

            Button(
                onClick = { viewModel.exportDataToCsv() },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.secondary)
            ) {
                Text("Exportar Últimos 30 Minutos (CSV)")
            }
        }
    }
}