package com.airstation.app.data.repository

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.floatPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map
import javax.inject.Inject
import javax.inject.Singleton

// Cria a extensão para acessar o DataStore
private val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "user_settings")

@Singleton
class UserPreferencesRepository @Inject constructor(
    @ApplicationContext private val context: Context
) {

    // Chaves de Configuração
    private object Keys {
        val NOTIFICATIONS_ENABLED = booleanPreferencesKey("notifications_enabled")
        val PM25_THRESHOLD = floatPreferencesKey("pm25_threshold") // Limite de alerta PM2.5
        val CO_THRESHOLD = floatPreferencesKey("co_threshold")     // Limite de alerta CO (CORREÇÃO AQUI)
    }

    // --- LEITURA (Flow) ---

    // Retorna se notificações estão ativadas (Padrão: true)
    val notificationsEnabled: Flow<Boolean> = context.dataStore.data
        .map { prefs -> prefs[Keys.NOTIFICATIONS_ENABLED] ?: true }

    // Retorna o limite do PM2.5 (Padrão: 35.0 - Nível OMS)
    val pm25Threshold: Flow<Float> = context.dataStore.data
        .map { prefs -> prefs[Keys.PM25_THRESHOLD] ?: 35.0f }

    // Retorna o limite do CO (Padrão: 50.0 ppm)
    val coThreshold: Flow<Float> = context.dataStore.data
        .map { prefs -> prefs[Keys.CO_THRESHOLD] ?: 50.0f }

    // --- ESCRITA (Suspend) ---

    suspend fun setNotificationsEnabled(enabled: Boolean) {
        context.dataStore.edit { prefs ->
            prefs[Keys.NOTIFICATIONS_ENABLED] = enabled
        }
    }

    suspend fun setPm25Threshold(value: Float) {
        context.dataStore.edit { prefs ->
            prefs[Keys.PM25_THRESHOLD] = value
        }
    }

    suspend fun setCoThreshold(value: Float) {
        context.dataStore.edit { prefs ->
            prefs[Keys.CO_THRESHOLD] = value
        }
    }
}