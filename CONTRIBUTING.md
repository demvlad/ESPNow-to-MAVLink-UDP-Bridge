## Using Git and GitHub

If you need help with pull requests, GitHub has excellent guides:
- [Creating a pull request](https://help.github.com/articles/creating-a-pull-request/)
- [GitHub Flow](https://guides.github.com/introduction/flow/)

### Step-by-Step Contribution Process

1. **Fork the Repository**
   - Login to GitHub
   - Go to [ESPNow-to-MAVLink-UDP-Bridge](https://github.com/demvlad/ESPNow-to-MAVLink-UDP-Bridge)
   - Click **"Fork"** (top-right corner)

2. **Clone Your Fork**

   git clone https://github.com/YOUR-USERNAME/ESPNow-to-MAVLink-UDP-Bridge.git
   
   cd ESPNow-to-MAVLink-UDP-Bridge

3. **Set Upstream Remote**

   git remote add upstream https://github.com/demvlad/ESPNow-to-MAVLink-UDP-Bridge.git

4. **Create a Feature Branch**

   git checkout main
   
   git pull origin main
   
   git checkout -b feature/your-feature-name

5. **Make and Test Changes**
   
7. **Commit Your Changes**

   git add .
   
   git commit -m "type(scope): brief description"

9. **Push to Your Fork**

   git push origin feature/your-feature-name

10. **Create Pull Request**

   - Go to your fork on GitHub

   - Click "Compare & pull request"

   - Ensure proper branch selection:

      - base: demvlad/ESPNow-to-MAVLink-UDP-Bridge:main
   
      - head: your-username:feature/your-feature-name
    
   - Complete the PR template and submit


**For Subsequent Contributions**

Repeat from step 4 for each new change. Always create a fresh branch.


