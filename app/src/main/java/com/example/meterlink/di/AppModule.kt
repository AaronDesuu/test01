package com.example.meterlink.di

import android.content.Context
import com.example.meterlink.data.repository.BleRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {

    @Provides
    @Singleton
    fun provideBleRepository(@ApplicationContext context: Context): BleRepository {
        return BleRepository(context)
    }
}