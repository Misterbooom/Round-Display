import MaterialIcons from '@expo/vector-icons/MaterialIcons'
import { useEffect, useRef, useState } from 'react'
import {
  Animated,
  Easing,
  Pressable,
  StyleSheet,
  Switch,
  View,
} from 'react-native'
import AnimatedReanimated, {
  FadeIn,
  FadeInUp,
} from 'react-native-reanimated'

import { ThemedText } from '@/components/themed-text'
import { ThemedView } from '@/components/themed-view'
import { typography } from '@/constants/theme'
import { useThemeColor } from '@/hooks/use-theme-color'

// ── Scanning Hero Card ───────────────────────────────────

function ScanningCard() {
  const primaryColor = useThemeColor({}, 'primary')
  const borderColor = useThemeColor({}, 'border')
  const textSecondary = useThemeColor({}, 'textSecondary')

  const rotate = useRef(new Animated.Value(0)).current
  const rotateReverse = useRef(new Animated.Value(0)).current
  const pulse = useRef(new Animated.Value(1)).current

  useEffect(() => {
    const loop = Animated.loop(
      Animated.timing(rotate, {
        toValue: 1,
        duration: 4000,
        easing: Easing.linear,
        useNativeDriver: true,
      }),
    )
    loop.start()
    return () => loop.stop()
  }, [rotate])

  useEffect(() => {
    const loop = Animated.loop(
      Animated.timing(rotateReverse, {
        toValue: 1,
        duration: 5000,
        easing: Easing.linear,
        useNativeDriver: true,
      }),
    )
    loop.start()
    return () => loop.stop()
  }, [rotateReverse])

  useEffect(() => {
    const loop = Animated.loop(
      Animated.sequence([
        Animated.timing(pulse, {
          toValue: 0.88,
          duration: 1000,
          easing: Easing.inOut(Easing.ease),
          useNativeDriver: true,
        }),
        Animated.timing(pulse, {
          toValue: 1,
          duration: 1000,
          easing: Easing.inOut(Easing.ease),
          useNativeDriver: true,
        }),
      ]),
    )
    loop.start()
    return () => loop.stop()
  }, [pulse])

  const spin = rotate.interpolate({
    inputRange: [0, 1],
    outputRange: ['0deg', '360deg'],
  })
  const spinReverse = rotateReverse.interpolate({
    inputRange: [0, 1],
    outputRange: ['360deg', '0deg'],
  })

  return (
    <ThemedView
      color="surfaceContainer"
      style={[styles.glassCard, { borderColor }]}
    >
      {/* Rotating rings — absolutely positioned behind content */}
      <View style={styles.ringsContainer}>
        <Animated.View
          style={[
            styles.ring,
            styles.ringOuter,
            { borderColor },
            { transform: [{ rotate: spin }] },
          ]}
        />
        <Animated.View
          style={[
            styles.ring,
            styles.ringInner,
            { borderColor },
            { transform: [{ rotate: spinReverse }] },
          ]}
        />
      </View>

      {/* Center content */}
      <View style={styles.scanContent}>
        <Animated.View
          style={[
            styles.scanIconCircle,
            {
              backgroundColor: primaryColor + '15',
              borderColor: primaryColor + '30',
              transform: [{ scale: pulse }],
            },
          ]}
        >
          <MaterialIcons name="bluetooth-searching" size={32} color={primaryColor} />
        </Animated.View>
        <ThemedText style={{ color: textSecondary, fontSize: 15 }}>
          Scanning for devices...
        </ThemedText>
      </View>
    </ThemedView>
  )
}

// ── Device Row ───────────────────────────────────────────

function DeviceRow({
  icon,
  name,
  status,
  onPress,
  delay = 0,
}: {
  icon: keyof typeof MaterialIcons.glyphMap
  name: string
  status: string
  onPress?: () => void
  delay?: number
}) {
  const borderColor = useThemeColor({}, 'border')
  const iconColor = useThemeColor({}, 'icon')
  const primaryColor = useThemeColor({}, 'primary')

  return (
    <AnimatedReanimated.View
      entering={FadeInUp.duration(400).delay(delay).springify()}
      style={{ width: '100%', alignItems: 'center' }}
    >
      <Pressable
        onPress={onPress}
        style={({ pressed }) => [
          styles.deviceCard,
          { borderColor },
          pressed && { opacity: 0.7 },
        ]}
      >
        <ThemedView
          color="surfaceContainer"
          style={[styles.deviceIconCircle, { borderColor }]}
        >
          <MaterialIcons name={icon} size={20} color={iconColor} />
        </ThemedView>
        <View style={styles.deviceInfo}>
          <ThemedText type="defaultSemiBold" style={{ fontSize: 15 }}>
            {name}
          </ThemedText>
          <ThemedText
            style={[
              styles.deviceStatusLabel,
              { color: primaryColor },
            ]}
          >
            {status}
          </ThemedText>
        </View>
        <MaterialIcons name="settings" size={20} color={iconColor} />
      </Pressable>
    </AnimatedReanimated.View>
  )
}

// ── Section Label ────────────────────────────────────────

function SectionLabel({ children }: { children: string }) {
  const textSecondary = useThemeColor({}, 'textSecondary')
  return (
    <ThemedText
      style={[
        {
          color: textSecondary,
          fontSize: typography.label.fontSize,
          fontFamily: typography.label.fontFamily,
          letterSpacing: typography.label.letterSpacing,
          textTransform: 'uppercase',
          paddingLeft: 4,
          marginBottom: 8,
        },
      ]}
    >
      {children}
    </ThemedText>
  )
}

// ── Settings Toggle Row ──────────────────────────────────

function ToggleRow({
  icon,
  label,
  value,
  onValueChange,
  delay = 0,
}: {
  icon: keyof typeof MaterialIcons.glyphMap
  label: string
  value: boolean
  onValueChange: (v: boolean) => void
  delay?: number
}) {
  const borderColor = useThemeColor({}, 'border')
  const iconColor = useThemeColor({}, 'icon')
  const primaryColor = useThemeColor({}, 'primary')
  const textColor = useThemeColor({}, 'icon')

  return (
    <AnimatedReanimated.View
      entering={FadeInUp.duration(400).delay(delay).springify()}
      style={{ width: '100%', alignItems: 'center' }}
    >
      <ThemedView
        color="surfaceContainer"
        style={[styles.toggleCard, { borderColor }]}
      >
        <MaterialIcons name={icon} size={20} color={iconColor} />
        <ThemedText
          style={[
            styles.toggleLabel,
            { color: textColor },
          ]}
        >
          {label}
        </ThemedText>
        <Switch
          value={value}
          onValueChange={onValueChange}
          trackColor={{ false: borderColor, true: primaryColor + '60' }}
          thumbColor={value ? primaryColor : iconColor}
          ios_backgroundColor={borderColor}
        />
      </ThemedView>
    </AnimatedReanimated.View>
  )
}

// ── Main Screen ──────────────────────────────────────────

export default function ConnectScreen() {
  const primaryColor = useThemeColor({}, 'primary')
  const borderColor = useThemeColor({}, 'border')

  const [autoConnect, setAutoConnect] = useState(true)
  const [keepAwake, setKeepAwake] = useState(false)

  return (
    <ThemedView style={styles.root}>
      {/* ── Hero: Scanning indicator ── */}
      <View style={styles.section}>
        <AnimatedReanimated.View
          entering={FadeIn.duration(600).springify()}
          style={{ alignItems: 'center' }}
        >
          <ThemedText
            type="title"
            style={{
              color: primaryColor,
              fontSize: typography.h2.fontSize,
              fontFamily: typography.h2.fontFamily,
              letterSpacing: -0.5,
              marginBottom: 16,
            }}
          >
            Connectivity
          </ThemedText>
          <ScanningCard />
        </AnimatedReanimated.View>
      </View>

      {/* ── Paired Devices ── */}
      <View style={styles.section}>
        <SectionLabel>Paired Device</SectionLabel>
        <DeviceRow
          icon="watch"
          name="Orbital Watch Series X"
          status="Connected"
          delay={200}
        />
      </View>

      {/* ── Settings ── */}
      <View style={styles.section}>
        <SectionLabel>Settings</SectionLabel>
        <ToggleRow
          icon="bluetooth-connected"
          label="Auto-connect"
          value={autoConnect}
          onValueChange={setAutoConnect}
          delay={350}
        />
        <View style={{ height: 10 }} />
        <ToggleRow
          icon="brightness-6"
          label="Keep screen on"
          value={keepAwake}
          onValueChange={setKeepAwake}
          delay={500}
        />
      </View>
    </ThemedView>
  )
}

// ── Styles ───────────────────────────────────────────────

const CARD_H = 200
const OUTER_RING = 220
const INNER_RING = 160

const styles = StyleSheet.create({
  root: {
    flex: 1,
    paddingHorizontal: 24,
  },
  section: {
    marginBottom: 24,
  },

  // ── Glass / scanning card ──
  glassCard: {
    width: '100%',
    height: CARD_H,
    borderRadius: 20,
    borderWidth: 1,
    alignItems: 'center',
    justifyContent: 'center',
    overflow: 'hidden',
  },
  ringsContainer: {
    ...StyleSheet.absoluteFillObject,
    justifyContent: 'center',
    alignItems: 'center',
  },
  ring: {
    position: 'absolute',
    borderRadius: 9999,
    borderWidth: 1,
    opacity: 0.12,
  },
  ringOuter: {
    width: OUTER_RING,
    height: OUTER_RING,
  },
  ringInner: {
    width: INNER_RING,
    height: INNER_RING,
  },

  // ── Center content ──
  scanContent: {
    alignItems: 'center',
    gap: 14,
    zIndex: 1,
  },
  scanIconCircle: {
    width: 64,
    height: 64,
    borderRadius: 32,
    borderWidth: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },

  // ── Device card ──
  deviceCard: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
    width: '100%',
    paddingVertical: 14,
    paddingHorizontal: 16,
    borderRadius: 16,
    borderWidth: 1,
    backgroundColor: 'transparent', // ThemedView provides bg
  },
  deviceIconCircle: {
    width: 40,
    height: 40,
    borderRadius: 20,
    borderWidth: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  deviceInfo: {
    flex: 1,
  },
  deviceStatusLabel: {
    fontSize: 12,
    fontFamily: 'Montserrat_600SemiBold',
    textTransform: 'uppercase',
    letterSpacing: 0.5,
    marginTop: 2,
  },

  // ── Toggle card ──
  toggleCard: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 10,
    width: '100%',
    paddingVertical: 14,
    paddingHorizontal: 16,
    borderRadius: 16,
    borderWidth: 1,
  },
  toggleLabel: {
    flex: 1,
    fontSize: 14,
    fontFamily: 'Montserrat_600SemiBold',
    textTransform: 'uppercase',
    letterSpacing: 0.4,
  },
})
