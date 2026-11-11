"""Check that the WebAssembly game starts in a browser and reacts to the mouse."""
import sys
import time

from playwright.sync_api import sync_playwright

URL = "http://localhost:8000/publictoiletmanager.html"

with sync_playwright() as p:
    browser = p.chromium.launch(channel="chrome")
    # The same size as the iframe on the website
    page = browser.new_page(viewport={"width": 998, "height": 686})
    errors = []
    page.on("pageerror", lambda e: errors.append(str(e)))
    page.on("console", lambda m: print("console:", m.text))
    page.goto(URL)
    time.sleep(15)
    page.screenshot(path="screenshot_1_start.png")
    start = page.screenshot()
    # Open the build panel
    page.mouse.move(24, 662)
    page.mouse.click(24, 662)
    time.sleep(1)
    page.screenshot(path="screenshot_2_build_panel.png")
    build_panel = page.screenshot()
    # Close the build panel, open the toilets
    page.mouse.click(24, 662)
    page.mouse.click(95, 662)
    time.sleep(1)
    # Save the game
    page.mouse.click(197, 662)
    time.sleep(1)
    page.screenshot(path="screenshot_3_saved.png")
    saved = page.evaluate("localStorage.getItem('PublicToiletManager_SaveGame')")
    browser.close()

if errors:
    sys.exit(f"Page errors: {errors}")
if start == build_panel:
    sys.exit("Clicking the build button did not show the build panel")
if not saved or "Game Loaded" not in saved:
    sys.exit("Saving the game did not save it to local storage")
print("OK")
