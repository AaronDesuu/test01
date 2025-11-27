package com.example.meterlink.di

import android.content.Context
import com.example.meterlink.data.repository.BleRepository
import com.example.meterlink.data.repository.DlmsRepository
import com.example.meterlink.dlms.DLMS
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

    @Provides
    @Singleton
    fun provideDlmsProtocol(@ApplicationContext context: Context): DLMS {
        return DLMS(context)
    }

    @Provides
    @Singleton
    fun provideDlmsRepository(
        bleRepository: BleRepository,
        dlms: DLMS
    ): DlmsRepository {
        return DlmsRepository(bleRepository, dlms)
    }
}