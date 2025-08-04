# Bobcat Ignition Controller Web Interface

## Project Overview

This project is a **web-based ignition key interface** that simulates the ignition system of a Bobcat tractor/heavy equipment. The interface provides a realistic, interactive ignition key that users can rotate clockwise through different operational states.

## Purpose

The web interface serves as a digital replica of a physical ignition switch, allowing users to experience the sequence of starting heavy equipment through a web browser. This could be used for:
- Training simulations
- Equipment familiarization
- Digital dashboards
- Educational purposes
- Equipment status monitoring

## Key Requirements & Specifications

### 1. **Clock-Based Positioning System**
The ignition positions are arranged like a clock face:
- **10 o'clock position**: OFF state (-60°)
- **12 o'clock position**: ON state (0°)
- **1 o'clock position**: GLOW state (30°)
- **3 o'clock position**: START state (90°)

### 2. **Visual Design Requirements**
- **Key Orientation**: The key pivots centered and its top points **upward** from its pivot point aligning with the label that indicates its current state.
- **Label Alignment**: Position labels where the key tip actually points when rotated to each state
- **Smooth Movement**: No jerky transitions - the key should rotate smoothly and continuously
- **Realistic Appearance**: Golden/brass colored key with proper proportions
- **Professional Interface**: Clean, industrial look suitable for heavy equipment

### 3. **Interaction Behavior**
- **Clockwise Only**: Key can be turned clockwise or to turn off anti-clockwise
- **Drag to Rotate**: Click and drag the key to rotate it through positions
- **Snap to Position**: Key snaps to exact positions when released
- **START Spring-Back**: When released at START position, automatically returns to ON after a brief delay (simulating ignition crank)
- **Progressive Movement**: Must go through each state in sequence (OFF→ON→GLOW→START)

### 4. **State Management**
Each position has distinct characteristics:
- **OFF**: Gray color, system disabled
- **ON**: Green color, electrical systems active
- **GLOW**: Orange color, glow plugs heating (diesel engines)
- **START**: Red color, engine cranking

### 5. **Audio Feedback**
- Click sounds for normal transitions
- Special sound for GLOW state
- Engine cranking sound for START state
- Sounds should be realistic and appropriate for heavy equipment

### 6. **Technical Implementation**
- **Pure HTML/CSS/JavaScript**: No external frameworks required
- **Responsive Design**: Works on desktop and mobile devices
- **Touch Support**: Proper touch event handling for mobile devices
- **Smooth Animations**: CSS transitions with appropriate easing
- **Accessible**: Proper cursor states and user feedback

## Current File Structure

```
/docs/ignition/
├── index.html          # Main HTML structure
├── style.css           # CSS styling and layout
├── script.js           # JavaScript interaction logic
└── PROJECT_DESCRIPTION.md  # This documentation file
```

## Technical Architecture

### HTML Structure
- Main container with ignition assembly
- Bezel and face elements for the switch housing
- Key element that rotates
- Status display showing current state
- Audio elements for sound effects

### CSS Styling
- CSS custom properties for easy theming
- Flexbox layout for centering
- CSS transforms for key rotation
- Gradient backgrounds for realistic metallic appearance
- Responsive design considerations

### JavaScript Logic
- Event handling for mouse and touch interactions
- Angle calculation based on cursor position
- State management and validation
- Smooth animation controls
- Audio playback coordination

## Known Issues & Challenges

### 1. **Label Alignment Problem**
The most critical issue is ensuring labels (OFF, ON, GLOW, START) are positioned exactly where the key tip points when rotated to each state. Since the key points downward, the labels need to be positioned around the bottom and right edges of the ignition face.

### 2. **Smooth Rotation**
The key must rotate smoothly during dragging without jerky movements or sudden jumps between states. This requires careful coordinate-to-angle conversion and proper event handling.

### 3. **Mobile Touch Support**
Touch events need to be handled properly for mobile devices, including preventing default behaviors and handling touch coordinates correctly.

## Design Specifications

### Colors
- **Background**: Dark blue-gray (#2c3e50)
- **Bezel**: Metallic silver gradient
- **Face**: Dark charcoal (#2a2a2a)
- **Key**: Golden brass (#d4af37)
- **OFF State**: Gray (#888888)
- **ON State**: Green (#4caf50)
- **GLOW State**: Orange (#ff9800)
- **START State**: Red (#f44336)

### Dimensions
- **Switch Assembly**: 300px diameter
- **Key**: 120px length
- **Bezel Depth**: Realistic 3D appearance with shadows
- **Labels**: 18px font size, bold, uppercase

### Animation Timing
- **Drag Response**: Immediate (no transition during drag)
- **Snap Animation**: 0.2s ease-out
- **Spring-back Delay**: 300ms pause before returning from START to ON

## Future Enhancements

1. **Additional States**: Could add more positions for different equipment types
2. **Digital Integration**: Connect to real equipment status via APIs
3. **Customization**: Allow different key styles, colors, and positions
4. **Multiple Equipment**: Support for different vehicle types with different ignition sequences
5. **Sound Customization**: Different sound packs for different equipment brands

## Development Guidelines

When recreating or modifying this interface:

1. **Start with positioning**: Get the clock-based angles correct first
2. **Test label alignment**: Ensure labels appear where the key actually points
3. **Implement smooth dragging**: Focus on continuous rotation rather than discrete state jumps
4. **Mobile testing**: Always test touch interactions on mobile devices
5. **Visual polish**: The interface should look professional and realistic

## Success Criteria

The interface is successful when:
- ✅ Key rotates smoothly without jerky movements
- ✅ Labels are positioned exactly where the key points
- ✅ Clockwise-only progression works correctly
- ✅ START position springs back to ON automatically
- ✅ Visual design looks professional and realistic
- ✅ Works equally well on desktop and mobile
- ✅ Audio feedback is appropriate and working
- ✅ All interactions feel natural and intuitive

---

**Last Updated**: August 4, 2025
**Project Type**: Web Interface Simulation
**Technology Stack**: HTML5, CSS3, Vanilla JavaScript
**Target Audience**: Heavy equipment operators, trainees, simulators
