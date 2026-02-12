package com.airstation.app.di

import android.content.Context
import androidx.room.Room
import com.airstation.app.data.local.AppDatabase
import com.airstation.app.data.local.ReadingDao
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {

    // 1. Ensina o Hilt a criar o Banco de Dados
    @Provides
    @Singleton
    fun provideDatabase(@ApplicationContext context: Context): AppDatabase {
        return Room.databaseBuilder(
            context,
            AppDatabase::class.java,
            "air_station_db"
        )
            .fallbackToDestructiveMigration() // Opcional: Recria o banco se mudar a versão
            .build()
    }

    // 2. Ensina o Hilt a extrair o DAO do Banco
    // O erro "MissingBinding ReadingDao" acontece porque faltava esta função!
    @Provides
    fun provideReadingDao(db: AppDatabase): ReadingDao {
        return db.readingDao()
    }
}